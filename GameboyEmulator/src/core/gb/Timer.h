#include <cstdint>
#include <array>



namespace gbemu {

	class Timer {
	public:
		Timer() {}

		void update()
		{
			++m_Counter;
			bool currentFreqBit = (m_Counter & m_FrequencyBitMask[TAC.Freqency]);
			if (m_FrequencyBitWasSet && !(currentFreqBit && TAC.Enable)) //Falling edge
			{
				//TODO: detect overflow, trigger interrupt
				++TIMA;
			}

			m_FrequencyBitWasSet = currentFreqBit && TAC.Enable;
		}
		uint8_t read(uint16_t address)
		{
			switch (address)
			{
			case 0xFF04: return DIV;
			case 0xFF05: return TIMA;
			case 0xFF06: return TMA;
			case 0xFF07: return (uint8_t(TAC.Enable) << 2) + TAC.Freqency;
			}
		}
		void write(uint16_t address, uint8_t data)
		{
			switch (address) {
			case 0xFF04:
			{
				m_Counter = 0;
				break;
			}
			case 0xFF05:
			{
				TIMA = data;
				break;
			}
			case 0xFF06:
			{
				TMA = data;
				break;
			}
			case 0xFF07:
			{
				TAC.Enable = (data & 0b00000100) != 0;
				TAC.Freqency = data & 0b00000011;
				break;
			}
			}
		}

	private:

		union {
			uint16_t m_Counter{ 0 };

			struct{
				uint8_t align;
				uint8_t DIV;
			};
		};

		uint8_t TIMA{ 0 };
		uint8_t TMA{ 0 };


		struct {
			uint8_t Freqency : 2;
			uint8_t Enable : 1;
		} TAC;

		bool m_FrequencyBitWasSet{ false };

		//Frequencies of a timer: 4096 Hz, 262144 Hz, 65536 Hz and 16386 Hz respectively. Are set from bits 0-1 of TAC
		const std::array<uint16_t, 4> m_FrequencyBitMask = { 1 << 9, 1 << 3, 1 << 5, 1 << 7 };
	};
}