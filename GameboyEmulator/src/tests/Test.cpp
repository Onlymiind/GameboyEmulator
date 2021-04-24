#include "tests/Test.h"

#include "imgui.h"
#include "imgui-SFML.h"

namespace gbemu {




	void Menu::update()
	{
		if (p_Test) p_Test->update();
	}

	void Menu::updateGUI()
	{
		ImGui::Begin("Test window");
		if (p_Test)
		{
			p_Test->updateGUI();
			if (ImGui::Button("Back")) p_Test.release();
		}
		else
		{
			for (const auto& [name, constructor] : m_TestBase)
			{
				if (ImGui::Button(name.data()))
				{
					p_Test.reset(constructor());
				}
			}
		}
		ImGui::End();
	}

	void Menu::render()
	{

		//ImGui::SFML::Update(window);

	}

}