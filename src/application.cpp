#include "application.h"
#include "breakpoint.h"
#include "disassembler.h"
#include "gb/address_bus.h"
#include "gb/cpu/cpu.h"
#include "gb/cpu/cpu_utils.h"
#include "gb/cpu/decoder.h"
#include "gb/cpu/operation.h"
#include "gb/emulator.h"
#include "gb/memory/basic_components.h"
#include "gb/ppu/ppu.h"
#include "gb/timer.h"
#include "imgui_internal.h"
#include "renderer.h"
#include "util/util.h"

#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <string>

namespace emulator {
    void Application::draw() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("name", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_MenuBar);

        drawMainMenu();

        if (ImGui::BeginTabBar("View")) {
            if (ImGui::BeginTabItem("Emulator view")) {
                drawEmulatorView();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debugger")) {
                drawDebuggerMenu();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Memory view")) {
                drawMemoryView();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();

        // ImGui::ShowDemoWindow();

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

                if (ImGui::InputText("###newdir", &new_romdir_, ImGuiInputTextFlags_EnterReturnsTrue)) {
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

    void Application::drawDebuggerMenu() {
        ImGui::BeginTable("##table", 2);
        ImGui::TableNextColumn();
        for (size_t i = 0; i < recent_instructions_.size(); ++i) {
            buffer_.clear();
            printInstruction(buffer_, recent_instructions_[i], i);
            if (ImGui::Selectable(buffer_.data())) {
                registers_to_print_ = std::pair<gb::cpu::RegisterFile, bool>{recent_instructions_[i].registers,
                                                                             recent_instructions_[i].ime};
            }
        }

        if (registers_to_print_) {
            ImGui::NewLine();
            auto [regs, ime] = *registers_to_print_;
            buffer_.clear();
            buffer_.reserve(g_registers_buffer_size);
            buffer_.putString("Carry: ")
                .putBool(regs.getFlag(gb::cpu::Flags::CARRY))
                .putString(", Half carry: ")
                .putBool(regs.getFlag(gb::cpu::Flags::HALF_CARRY))
                .putString("\nNegative: ")
                .putBool(regs.getFlag(gb::cpu::Flags::NEGATIVE))
                .putString(", Zero: ")
                .putBool(regs.getFlag(gb::cpu::Flags::ZERO))
                .putString("\nA: ")
                .putU8(regs.A())
                .putString(", AF: ")
                .putU16(regs.AF())
                .putString("\nC: ")
                .putU8(regs.C())
                .putString(", B: ")
                .putU8(regs.B())
                .putString(", BC: ")
                .putU16(regs.BC())
                .putString("\nE: ")
                .putU8(regs.E())
                .putString(", D: ")
                .putU8(regs.D())
                .putString(", DE: ")
                .putU16(regs.DE())
                .putString("\nH: ")
                .putU8(regs.H())
                .putString(", L: ")
                .putU8(regs.L())
                .putString(", HL: ")
                .putU16(regs.HL())
                .putString("\nSP: ")
                .putU16(regs.sp)
                .putString(", PC: ")
                .putU16(regs.PC())
                .putString("\nIME: ")
                .putBool(ime);
            buffer_.finish();
            ImGui::TextUnformatted(buffer_.data(), buffer_.data() + buffer_.size());
        }

        ImGui::NewLine();
        uint64_t instr = 0;
        ImGui::TextUnformatted("Fast forward (in number of instructions):");
        if (ImGui::InputScalar("##run_instr", ImGuiDataType_U64, &instr, nullptr, nullptr, "%d",
                               ImGuiInputTextFlags_EnterReturnsTrue)) {
            for (uint64_t i = 0; i < instr; ++i) {
                update();
                while (!emulator_.terminated() && !emulator_.getCPU().isFinished()) {
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

        drawDisassembly();
    }

    void Application::drawDisassembly() {
        if (disassembler_.isDirty()) {
            InstructionAddress next_addr{.address = 0xffff};
            disasm_buffer_.clear();
            disassembly_line_count_ = disassembler_.size();
            size_t offset = 0;
            InstructionAddress last_address;
            for (auto instr : disassembler_) {
                if (next_addr != instr.first) {
                    size_t offset_change = 0;
                    if (next_addr.bank != instr.first.bank) {
                        disasm_buffer_.reserve(sizeof("\nBank ffff:\n"));
                        if (gb::g_memory_rom.isInRange(instr.first.address) ||
                            gb::g_memory_cartridge_ram.isInRange(instr.first.address)) {
                            disasm_buffer_.putString("\nBank ").putU16(instr.first.bank).putString(":\n");
                        } else {
                            disasm_buffer_.putString("\nNot banked:\n");
                        }
                        offset_change += 2;
                    }
                    disasm_buffer_.reserve(sizeof("\nffff:\n"));
                    disasm_buffer_.put('\n').putU16(instr.first.address);
                    disasm_buffer_.putString(":\n");
                    offset_change += 2;
                    offset += offset_change;
                    instruction_line_offsets_[instr.first] = offset;
                }
                printInstruction(disasm_buffer_, instr.second);
                disasm_buffer_.put('\n');
                next_addr = InstructionAddress{uint16_t(instr.first.address + instr.second.width), instr.first.bank};
                last_address = instr.first;
            }
            disassembly_line_count_ += offset;
            instruction_line_offsets_[last_address] = offset;
            disasm_buffer_.finish();
            disassembler_.clearDirtyFlag();
        }

        if (ImGui::BeginChild("##disassembly")) {
            ImGui::TextUnformatted(disasm_buffer_.data(), disasm_buffer_.data() + disasm_buffer_.size());
            if ((ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) &&
                ImGui::IsKeyDown(ImGuiKey_F)) {
                ImGui::OpenPopup("Search instruction by address");
            }
            std::optional<float> scroll;
            if (ImGui::BeginPopup("Search instruction by address")) {
                ImGui::InputScalar("Address##search", ImGuiDataType_U16, &search_instruction_address_, nullptr, nullptr,
                                   "%.4x", ImGuiInputTextFlags_CharsHexadecimal);
                ImGui::InputScalar("Bank##search", ImGuiDataType_U16, &search_instruction_bank_, nullptr, nullptr,
                                   "%.4x", ImGuiInputTextFlags_CharsHexadecimal);
                if (ImGui::Button("Search")) {

                    InstructionAddress addr{
                        search_instruction_address_,
                        search_instruction_bank_,
                    };
                    auto it = disassembler_.at(addr);
                    if (it != disassembler_.end()) {
                        size_t line = std::distance(disassembler_.begin(), it) + 1;
                        size_t offset = 0;
                        if (auto offset_it = instruction_line_offsets_.upper_bound(addr);
                            offset_it != instruction_line_offsets_.begin()) {
                            --offset_it;
                            offset = offset_it->second;
                        }
                        line += offset;

                        scroll = std::min(float(line) / float(disassembly_line_count_), 1.0f);
                    }
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
                if (scroll) {
                    ImGui::SetScrollHereY(*scroll);
                    scroll = std::nullopt;
                }
            }
            ImGui::EndChild();
        }
    }

    void Application::drawBreakpointMenu() {
        uint16_t pc_break = 0;
        ImGui::TextUnformatted("Add PC breakpoint:");
        if (ImGui::InputScalar("##Add PC breakpoint input", ImGuiDataType_U16, &pc_break, nullptr, nullptr, "%.4x",
                               ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            addPCBreakpoint(pc_break);
        }
        {
            ImGui::Text("PC breakpoints: ");
            auto delete_it = pc_breakpoints_.end();
            for (auto it = pc_breakpoints_.begin(); it != pc_breakpoints_.end(); ++it) {
                buffer_.clear();
                buffer_.reserve(6);
                buffer_.putString("0x").putU16(*it);
                buffer_.finish();
                ImGui::Selectable(buffer_.data());
                if (ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                    delete_it = it;
                }
            }
            if (delete_it != pc_breakpoints_.end()) {
                pc_breakpoints_.erase(delete_it);
            }
        }

        ImGui::TextUnformatted("Add memory breakpoint:");
        ImGui::InputScalar("##address", ImGuiDataType_U16, &memory_breakpoint_data_.address, nullptr, nullptr, "%.4x",
                           ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::TextUnformatted("Breakpoint mode:");
        if (ImGui::BeginCombo("##break_on", to_string(memory_breakpoint_data_.break_on).data())) {
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
            for (auto br : memory_breakpoints_.getBreakpoints()) {
                buffer_.clear();
                buffer_.reserve(sizeof("Address: 0xffff\nBreak on: ALWAYS, value: 0xff"));
                buffer_.putString("Address: 0x")
                    .putU16(br.address)
                    .putString("\nbreak on: ")
                    .putString(to_string(br.break_on));
                if (br.value) {
                    buffer_.putString(", value: 0x").putU8(*br.value);
                }
                buffer_.finish();
                ImGui::Selectable(buffer_.data());
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

        if (ImGui::BeginTabBar("View memory")) {
            using enum gb::MemoryObjectType;
            gb::MemoryObjectType memory_to_display = ROM;
            if (ImGui::BeginTabItem("ROM")) {
                drawMemoryRegion(ROM);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("VRAM")) {
                drawMemoryRegion(VRAM);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Cartridge RAM")) {
                drawMemoryRegion(CARTRIDGE_RAM);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("WRAM")) {
                drawMemoryRegion(WRAM);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("OAM")) {
                drawMemoryRegion(OAM);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("IO")) {
                drawMemoryRegion(IO);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("HRAM")) {
                drawMemoryRegion(HRAM);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Interrupt Enable")) {
                drawMemoryRegion(IE);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

    void Application::drawMemoryRegion(gb::MemoryObjectType region) {
        constexpr std::string_view row_fmt = "0x%.4x: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x "
                                             "%.2x %.2x %.2x %.2x %.2x";

        gb::MemoryObjectInfo info = gb::objectTypeToInfo(region);
        uint16_t len = info.max_address - info.min_address + 1;
        if ((region == gb::MemoryObjectType::ROM && !emulator_.getCartridge().hasROM()) ||
            (region == gb::MemoryObjectType::CARTRIDGE_RAM && !emulator_.getCartridge().hasRAM())) {
            len = 0;
        }
        uint16_t i = 0;
        size_t rows = info.size / 16 + (info.size % 16 != 0);
        buffer_.clear();
        buffer_.reserve(rows * (16 * 3 + 8));
        for (; i + 15 < len; i += 16) {
            uint16_t base = info.min_address + i;
            buffer_.putString("0x")
                .putU16(base)
                .putString(": ")
                .putU8(*emulator_.peekMemory(base))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 1))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 2))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 3))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 4))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 5))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 6))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 7))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 8))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 9))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 10))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 11))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 12))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 13))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 14))
                .put(' ')
                .putU8(*emulator_.peekMemory(base + 15))
                .put('\n');
        }

        if (i < len) {
            buffer_.putString("0x").putU16(info.min_address + i).put(':');
            for (; i < len; ++i) {
                buffer_.put(' ').putU8(*emulator_.peekMemory(info.min_address + i));
            }
        }
        buffer_.finish();

        if (ImGui::BeginChild("##memory view")) {
            ImGui::TextUnformatted(buffer_.data(), buffer_.data() + buffer_.size());
        }
        ImGui::EndChild();
    }

    void Application::drawEmulatorView() {
        emulator_renderer_->flush();
        ImVec2 position = ImGui::GetCursorScreenPos();
        ImVec2 img_size = ImGui::GetContentRegionAvail();
        img_size.y = img_size.x * (float(gb::g_screen_height) / float(gb::g_screen_width));
        ImGui::Image((void *)emulator_renderer_->getTextureID(), img_size);
        if (ImGui::IsItemHovered()) {
            float scale = img_size.x / gb::g_screen_width;
            ImVec2 mouse_coords = ImGui::GetMousePos();
            mouse_coords.x -= position.x;
            mouse_coords.y -= position.y;
            float tile_dimension = 8 * scale;

            ImVec2 rect_min(std::floor(mouse_coords.x / (8 * scale)) * tile_dimension,
                            std::floor(mouse_coords.y / (8 * scale)) * tile_dimension);
            rect_min.x += position.x;
            rect_min.y += position.y;
            ImVec2 rect_max(rect_min.x + tile_dimension, rect_min.y + tile_dimension);
            ImGui::GetWindowDrawList()->AddRect(rect_min, rect_max, IM_COL32(255, 0, 0, 255));
        }
    }

    Application::Application() : memory_breakpoints_([this]() { single_step_ = true; }) {
        initGUI();
        emulator_.getBus().setObserver(memory_breakpoints_);
    }

    void Application::run() {
        while (is_running_) {

            double start = glfwGetTime();

            if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
                single_step_ = !single_step_;
            }

            if (single_step_) {
                if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
                    update();
                    while (!(emulator_.getCPU().isFinished() || emulator_.terminated())) {
                        update();
                    }
                } else if (ImGui::IsKeyPressed(ImGuiKey_F12)) {
                    advanceFrame();
                }
            } else {
                advanceFrame();
            }

            glfwSetWindowTitle(window_, ("emulator [" + std::to_string(glfwGetTime() - start) + "]").c_str());

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
        window_ = glfwCreateWindow(1000, 1000, "emulator", nullptr, nullptr);
        glfwMakeContextCurrent(window_);
        if (gladLoadGL(glfwGetProcAddress) == 0) {
            std::cout << "failed to load OpenGL" << std::endl;
            is_running_ = false;
            return;
        }

        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init();
        refresh_rate_ = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
        emulator_renderer_ = std::make_unique<renderer::Renderer>();
        emulator_.getPPU().setRenderer(*emulator_renderer_);
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
            if (emulator_.getCPU().isFinished()) {
                gb::cpu::Instruction instr = emulator_.getCPU().getLastInstruction();
                recent_instructions_.push_back(instr);
                if (instr.registers.PC() <= gb::g_memory_rom_bank0_max_address) {
                    disassembler_.addInstruction(instr, current_rom_banks_.first);
                } else if (instr.registers.PC() <= gb::g_memory_rom.max_address) {
                    disassembler_.addInstruction(instr, current_rom_banks_.second);
                } else if (gb::g_memory_cartridge_ram.isInRange(instr.registers.PC())) {
                    disassembler_.addInstruction(instr, current_ram_bank_);
                } else {
                    disassembler_.addInstruction(instr);
                }
                current_ram_bank_ = emulator_.getCartridge().getCurrentRAMBank();
                current_rom_banks_ = emulator_.getCartridge().getCurrentROMBanks();
                // StaticStringBuffer<g_instruction_string_buf_size> buf;
                // printInstruction(buf, recent_instructions_.size() - 1);
                // std::cout << buf.data() << '\n';
                if (!single_step_ &&
                    std::binary_search(pc_breakpoints_.begin(), pc_breakpoints_.end(), instr.registers.PC())) {
                    single_step_ = true;
                }
            }
        } catch (const std::exception &e) {
            std::cout << "exception occured during emulator update: " << e.what() << std::endl;
            emulator_.stop();
            single_step_ = true;
        }
    }

    void Application::advanceFrame() {
        if (emulator_.terminated()) {
            return;
        }
        bool old_single_step = single_step_;
        single_step_ = false;
        constexpr size_t clock_frequency = 1 << 20;
        size_t max_updates = (clock_frequency / refresh_rate_) * 2;

        update();
        while (!emulator_.getPPU().frameFinished() && max_updates > 0) {
            if (single_step_) {
                // run current instruction until completion
                while (!emulator_.getCPU().isFinished()) {
                    update();
                }
                break;
            }
            update();
            --max_updates;
        }
        emulator_.getPPU().resetFrameFinistedFlag();
        if (!single_step_) {
            single_step_ = old_single_step;
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

        emulator_.getCartridge().setROM(std::move(data));
        emulator_.reset();
        emulator_.start();
        disassembler_.clear();

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

    void Application::printInstruction(StringBuffer &buf, gb::cpu::Instruction instr, std::optional<size_t> idx) {
        using namespace gb::cpu;
        buf.reserve(sizeof("ffff CALL nz, ffff##125"));
        buf.putU16(instr.registers.PC()).put(' ').putString(to_string(instr.type));
        if (instr.condition) {
            buf.put(' ').putString(to_string(*instr.condition));
        }

        auto print_arg = [&buf](gb::cpu::Instruction::Argument arg) {
            if (arg.empty()) {
                return;
            }
            buf.put(' ');
            if (arg.is<Registers>()) {
                buf.putString(to_string(arg.get<Registers>()));
            } else if (arg.is<int8_t>()) {
                buf.putSigned(arg.get<int8_t>());
            } else if (arg.is<uint8_t>()) {
                buf.putU8(arg.get<uint8_t>());
            } else if (arg.is<uint16_t>()) {
                buf.putU16(arg.get<uint16_t>());
            }
        };

        if (instr.load_subtype) {
            if (*instr.load_subtype == LoadSubtype::LD_OFFSET_SP) {
                buf.putString(" HL,");
            } else if (*instr.load_subtype == LoadSubtype::LD_IO) {
                buf.put('H');
            }
        }

        print_arg(instr.dst);

        if (instr.load_subtype) {
            switch (*instr.load_subtype) {
            case LoadSubtype::LD_DEC:
                if (instr.dst.get<Registers>() == Registers::HL) {
                    buf.putString("--");
                }
                break;
            case LoadSubtype::LD_INC:
                if (instr.dst.get<Registers>() == Registers::HL) {
                    buf.putString("++");
                }
                break;
            }
        }

        if (!instr.dst.empty()) {
            buf.put(',');
        }
        print_arg(instr.src);

        if (instr.load_subtype) {
            switch (*instr.load_subtype) {
            case LoadSubtype::LD_DEC:
                if (instr.src.get<Registers>() == Registers::HL) {
                    buf.putString("--");
                }
                break;
            case LoadSubtype::LD_INC:
                if (instr.src.get<Registers>() == Registers::HL) {
                    buf.putString("++");
                }
                break;
            }
        }
        if (idx) {
            buf.putString("##").putU8(uint8_t(*idx));
        }
        buf.finish();
    }
} // namespace emulator
