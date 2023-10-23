#include "Application.h"
#include "Breakpoint.h"
#include "gb/AddressBus.h"
#include "gb/Emulator.h"
#include "gb/Timer.h"
#include "gb/cpu/CPU.h"
#include "gb/cpu/CPUUtils.h"
#include "gb/cpu/Decoder.h"
#include "gb/cpu/Operation.h"
#include "gb/memory/BasicComponents.h"
#include "utils/Utils.h"

#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

namespace emulator {
    void Application::draw() {
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("name", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);
        drawMainMenu();

        ImGui::BeginTable("##table", 2);
        ImGui::TableNextColumn();
        StringBuffer<g_instruction_string_buf_size> buf;
        for (size_t i = 0; i < recent_instructions_.size(); ++i) {
            printInstruction(buf, i);
            if (ImGui::Selectable(buf.data())) {
                registers_to_print_ = recent_instructions_[i].registers;
            }
        }

        if (registers_to_print_) {
            ImGui::NewLine();
            auto &regs = *registers_to_print_;
            ImGui::Text(g_regs_fmt, regs.getFlag(gb::cpu::Flags::CARRY),
                        regs.getFlag(gb::cpu::Flags::HALF_CARRY),
                        regs.getFlag(gb::cpu::Flags::NEGATIVE), regs.getFlag(gb::cpu::Flags::ZERO),
                        regs.A(), regs.AF(), regs.C(), regs.B(), regs.BC(), regs.E(), regs.D(),
                        regs.DE(), regs.H(), regs.L(), regs.HL(), regs.sp, regs.pc);
        }

        ImGui::NewLine();
        uint64_t instr = 0;
        if (ImGui::InputScalar("##run_instr", ImGuiDataType_U64, &instr, nullptr, nullptr, "%d",
                               ImGuiInputTextFlags_EnterReturnsTrue)) {
            for (uint64_t i = 0; i < instr; ++i) {
                update();
                while (!emulator_.terminated() && !emulator_.instructionFinished()) {
                    update();
                }
            }
        }

        if (single_step_) {
            ImGui::NewLine();
            ImGui::TextUnformatted("Single stepping");
        }

        ImGui::TableNextColumn();
        drawBreakpointMenu();
        ImGui::EndTable();

        drawMemoryView();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    void Application::drawMainMenu() {
        if (!ImGui::BeginMenuBar()) {
            return;
        }

        if (ImGui::BeginMenu("File")) {

            if (ImGui::BeginMenu("Change ROM Directory")) {

                if (ImGui::InputText("###newdir", &new_romdir_,
                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
                    if (!setROMDirectory()) {
                        std::cout << "failed to change ROM directory\n";
                    }
                    new_romdir_.clear();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Available ROMs")) {
                if (!roms_.empty()) {
                    ImGui::TextWrapped("%s", roms_[0].parent_path().c_str());
                }
                for (const auto &rom : roms_) {
                    if (ImGui::Selectable(rom.filename().c_str())) {
                        if (!runROM(rom)) {
                            std::cout << "failed to run rom: " << rom << '\n';
                        }
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Recent ROMs")) {

                for (const auto &rom : recent_roms_) {
                    if (ImGui::Selectable(rom.filename().c_str())) {
                        if (!runROM(rom)) {
                            std::cout << "failed to run rom: " << rom << '\n';
                        }
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::Selectable("Stop execution")) {
                emulator_.stop();
            }

            if (ImGui::Selectable("Reset")) {
                recent_instructions_.clear();
                emulator_.reset();
            }

            if (ImGui::Selectable("Quit")) {
                is_running_ = false;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    void Application::drawBreakpointMenu() {
        uint16_t pc_break = 0;
        ImGui::TextUnformatted("Add PC breakpoint:");
        if (ImGui::InputScalar("###Add PC breakpoint input", ImGuiDataType_U16, &pc_break, nullptr,
                               nullptr, "%.4x",
                               ImGuiInputTextFlags_CharsHexadecimal |
                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
            addPCBreakpoint(pc_break);
        }
        {
            ImGui::Text("PC breakpoints: ");
            auto delete_it = pc_breakpoints_.end();
            std::string buf(6, '0');
            for (auto it = pc_breakpoints_.begin(); it != pc_breakpoints_.end(); ++it) {
                sprintf(buf.data(), "0x%.4x", *it);
                ImGui::Selectable(buf.c_str());
                if (ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                    delete_it = it;
                }
            }
            if (delete_it != pc_breakpoints_.end()) {
                pc_breakpoints_.erase(delete_it);
            }
        }

        ImGui::TextUnformatted("Add memory breakpoint:");
        ImGui::InputScalar("##address", ImGuiDataType_U16, &memory_breakpoint_data_.address,
                           nullptr, nullptr, "%.4x", ImGuiInputTextFlags_CharsHexadecimal);
        if (ImGui::BeginCombo("Break on:", to_string(memory_breakpoint_data_.break_on).data())) {
            if (ImGui::Selectable(to_string(MemoryBreakpointData::BreakOn::ALWAYS).data())) {
                memory_breakpoint_data_.break_on = MemoryBreakpointData::BreakOn::ALWAYS;
            } else if (ImGui::Selectable(to_string(MemoryBreakpointData::BreakOn::READ).data())) {
                memory_breakpoint_data_.break_on = MemoryBreakpointData::BreakOn::READ;
            } else if (ImGui::Selectable(to_string(MemoryBreakpointData::BreakOn::WRITE).data())) {
                memory_breakpoint_data_.break_on = MemoryBreakpointData::BreakOn::WRITE;
            }
            ImGui::EndCombo();
        }

        uint8_t temp_value = memory_breakpoint_data_.value ? *memory_breakpoint_data_.value : 0;

        // ImGui issue: InputScalar won't return true if the value is unchanged
        // this means that to set memory_breakpoint_data_.value to 0
        // you first need to input some non-zero value in GUI
        if (ImGui::InputScalar("value", ImGuiDataType_U8, &temp_value, nullptr, nullptr, "%.2x",
                               ImGuiInputTextFlags_CharsHexadecimal)) {
            memory_breakpoint_data_.value = temp_value;
        }

        if (ImGui::Button("Add")) {
            addMemoryBreakpoint();
        }
        {
            ImGui::Text("Memory breakpoints: ");
            std::optional<MemoryBreakpointData> delete_val;
            std::string buf(strlen("Address: 0xffff\nBreak on: ALWAYS, value: 0xff"), '\0');
            for (auto br : memory_breakpoints_.getBreakpoints()) {

                // it is fine to use %s to print to_string(br.break_on) since it is a string literal
                int written = sprintf(buf.data(), "Address: 0x%.4x\nBreak on: %s", br.address,
                                      to_string(br.break_on).data());
                if (br.value) {
                    sprintf(buf.data() + written, ", value: 0x%.2x", *br.value);
                }
                ImGui::Selectable(buf.c_str());
                if (ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                    delete_val = br;
                }
            }
            if (delete_val) {
                memory_breakpoints_.removeBreakpoint(*delete_val);
            }
        }
    }

    void Application::drawMemoryView() {
        constexpr std::string_view row_fmt =
            "0x%.4x: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x "
            "%.2x %.2x %.2x %.2x %.2x";

        ImGui::TextUnformatted("Memory range to display:");
        ImGui::InputScalarN(
            "##input mem range", ImGuiDataType_U16, mem_range_, 2, nullptr, nullptr, "%.4x",
            ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue);
        if (mem_range_[0] > mem_range_[1]) {
            mem_range_[1] = mem_range_[0];
        }

        uint16_t len = mem_range_[1] - mem_range_[0];
        uint16_t i = 0;
        for (; i + 15 < len; i += 16) {
            uint16_t base = mem_range_[0] + i;
            ImGui::Text(row_fmt.data(), base, emulator_.peekMemory(base),
                        emulator_.peekMemory(base + 1), emulator_.peekMemory(base + 2),
                        emulator_.peekMemory(base + 3), emulator_.peekMemory(base + 4),
                        emulator_.peekMemory(base + 5), emulator_.peekMemory(base + 6),
                        emulator_.peekMemory(base + 7), emulator_.peekMemory(base + 8),
                        emulator_.peekMemory(base + 9), emulator_.peekMemory(base + 10),
                        emulator_.peekMemory(base + 11), emulator_.peekMemory(base + 12),
                        emulator_.peekMemory(base + 13), emulator_.peekMemory(base + 14),
                        emulator_.peekMemory(base + 15));
        }

        if (i < len) {
            // account for null char!!
            size_t last_row_len = strlen("0xffff:") + (len - i) * strlen(" ff") + 1;
            std::string buf(last_row_len, '\0');
            int written = sprintf(buf.data(), "0x%.4x:", mem_range_[0] + i);
            for (; i < len; ++i) {
                written +=
                    sprintf(buf.data() + written, " %.2x", emulator_.peekMemory(mem_range_[0] + i));
            }
            ImGui::TextUnformatted(buf.data());
        }
    }

    Application::Application() {
        initGUI();
        emulator_.setMemoryObserver(memory_breakpoints_);
    }

    void Application::run() {
        size_t updates_per_frame = size_t(g_cycles_per_second / size_t(refresh_rate_));
        int frame = 0;
        while (is_running_) {
            double time_start = glfwGetTime();

            if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
                single_step_ = !single_step_;
            }

            if (single_step_ && ImGui::IsKeyPressed(ImGuiKey_S)) {
                update();
                while (!emulator_.instructionFinished()) {
                    update();
                }
            } else if (!single_step_) {
                for (int i = 0; !emulator_.terminated() && i < updates_per_frame; ++i) {
                    update();
                    if (single_step_) {
                        // run current instruction until completion
                        while (!emulator_.instructionFinished()) {
                            update();
                        }
                        break;
                    }
                }

                ++frame;
                if (frame == refresh_rate_) {
                    frame = 0;
                    size_t leftover_updates =
                        g_cycles_per_second - updates_per_frame * size_t(refresh_rate_);
                    for (int i = 0; !emulator_.terminated() && i < leftover_updates; ++i) {
                        update();
                    }
                }
            }

            draw();

            if (window_ && glfwWindowShouldClose(window_)) {
                is_running_ = false;
            }
        }
    }

    void Application::initGUI() {
        if (gui_init_) {
            return;
        }

        glfwInit();
        window_ = glfwCreateWindow(600, 600, "emulator", nullptr, nullptr);
        glfwMakeContextCurrent(window_);
        if (gladLoadGL() == 0) {
            std::cout << "failed to load OpenGL" << std::endl;
            is_running_ = false;
            return;
        }

        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init();
        // TODO: no support for multiple monitors
        refresh_rate_ = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
        gui_init_ = true;
    }

    Application::~Application() {
        if (gui_init_) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            glfwDestroyWindow(window_);
            glfwTerminate();
        }
    }

    void Application::update() {
        try {
            emulator_.tick();
            if (emulator_.instructionFinished()) {
                gb::cpu::Instruction instr = emulator_.getLastInstruction();
                recent_instructions_.push_back(instr);
                if (!single_step_ &&
                    std::binary_search(pc_breakpoints_.begin(), pc_breakpoints_.end(),
                                       instr.registers.pc)) {
                    single_step_ = true;
                }
            }
        } catch (const std::exception &e) {
            std::cout << "exception occured during emulator update: " << e.what() << std::endl;
        }
    }

    bool Application::setROMDirectory() {
        std::filesystem::path path(new_romdir_);
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            roms_.clear();
            for (const auto &item : std::filesystem::directory_iterator(path)) {
                if (item.is_regular_file() && item.path().extension() == g_rom_extension) {
                    roms_.push_back(item.path());
                }
            }
            return true;
        }

        return false;
    }

    bool Application::runROM(const std::filesystem::path path) {

        if (!std::filesystem::exists(path)) {
            return false;
        }

        std::vector<uint8_t> data = readFile(path);
        if (data.empty()) {
            return false;
        }

        pushRecent(recent_roms_, path);

        emulator_.setROM(std::move(data));
        emulator_.reset();
        emulator_.start();

        return true;
    }

    void Application::addPCBreakpoint(uint16_t address) {
        auto it = std::lower_bound(pc_breakpoints_.begin(), pc_breakpoints_.end(), address);
        pc_breakpoints_.insert(it, address);
    }

    void Application::addMemoryBreakpoint() {
        memory_breakpoints_.addBreakpoint(memory_breakpoint_data_);
        memory_breakpoint_data_ = MemoryBreakpointData{};
    }

    void Application::printInstruction(StringBuffer<g_instruction_string_buf_size> &buf,
                                       size_t idx) {
        using namespace gb::cpu;
        auto &instr = recent_instructions_[idx];
        std::stringstream out;
        out.rdbuf()->pubsetbuf(buf.data(), buf.capacityWithNullChar());

        out << std::hex << instr.registers.pc << ' ' << to_string(instr.type);
        if (instr.condition) {
            out << ' ' << to_string(*instr.condition);
        }

        auto print_arg = [&out](gb::cpu::Instruction::Argument arg) {
            if (arg.empty()) {
                return;
            } else if (arg.is<Registers>()) {
                out << ' ' << to_string(arg.get<Registers>());
            } else if (arg.is<int8_t>()) {
                out << std::setw(0) << std::dec << ' ' << (arg.get<int8_t>() > 0 ? "+" : "")
                    << +arg.get<int8_t>();
            } else if (arg.is<uint8_t>()) {
                out << ' ' << std::setw(2) << std::setfill('0') << std::hex
                    << int(arg.get<uint8_t>());
            } else if (arg.is<uint16_t>()) {
                out << ' ' << std::setw(4) << std::setfill('0') << std::hex
                    << int(arg.get<uint16_t>());
            }
        };

        if (instr.load_subtype) {
            if (*instr.load_subtype == LoadSubtype::LD_OFFSET_SP) {
                out << " HL,";
            } else if (*instr.load_subtype == LoadSubtype::LD_IO) {
                out << "H";
            }
        }

        print_arg(instr.dst);

        if (instr.load_subtype) {
            switch (*instr.load_subtype) {
            case LoadSubtype::LD_DEC:
                if (instr.dst.get<Registers>() == Registers::HL) {
                    out << "--";
                }
                break;
            case LoadSubtype::LD_INC:
                if (instr.dst.get<Registers>() == Registers::HL) {
                    out << "++";
                }
                break;
            }
        }

        if (!instr.dst.empty()) {
            out << ',';
        }
        print_arg(instr.src);

        if (instr.load_subtype) {
            switch (*instr.load_subtype) {
            case LoadSubtype::LD_DEC:
                if (instr.src.get<Registers>() == Registers::HL) {
                    out << "--";
                }
                break;
            case LoadSubtype::LD_INC:
                if (instr.src.get<Registers>() == Registers::HL) {
                    out << "++";
                }
                break;
            }
        }

        out << "##" << idx; // for Dear ImGui ids
    }
} // namespace emulator
