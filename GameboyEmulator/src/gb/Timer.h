#include <cstdint>
#include <array>



namespace gb {

	class Timer {
	public:
		Timer() {}

		void update()
		{
			++counter_;
			bool currentFreqBit = (counter_ & frequency_bit_mask_[TAC_.freqency]);
			if (frequency_bit_was_set_ && !(currentFreqBit && TAC_.enable)) //Falling edge
			{
				//TODO: detect overflow, trigger interrupt
				++TIMA_;
			}

			frequency_bit_was_set_ = currentFreqBit && TAC_.enable;
		}
		uint8_t read(uint16_t address)
		{
			switch (address)
			{
			case 0xFF04: return DIV_;
			case 0xFF05: return TIMA_;
			case 0xFF06: return TMA_;
			case 0xFF07: return (uint8_t(TAC_.enable) << 2) + TAC_.freqency;
			}
		}
		void write(uint16_t address, uint8_t data)
		{
			switch (address) {
			case 0xFF04:
			{
				counter_ = 0;
				break;
			}
			case 0xFF05:
			{
				TIMA_ = data;
				break;
			}
			case 0xFF06:
			{
				TMA_ = data;
				break;
			}
			case 0xFF07:
			{
				TAC_.enable = (data & 0b00000100) != 0;
				TAC_.freqency = data & 0b00000011;
				break;
			}
			}
		}

	private:

		union 
		{
			uint16_t counter_;

			struct
			{
				uint8_t align;
				uint8_t DIV_;
			};
		};

		uint8_t TIMA_ = 0;
		uint8_t TMA_ = 0;


		struct 
		{
			uint8_t freqency : 2;
			uint8_t enable : 1;
		} TAC_;

		bool frequency_bit_was_set_ = false;

		//Frequencies of a timer: 4096 Hz, 262144 Hz, 65536 Hz and 16386 Hz respectively. Are set from bits 0-1 of TAC
		const std::array<uint16_t, 4> frequency_bit_mask_ = { 1 << 9, 1 << 3, 1 << 5, 1 << 7 };
	};
}