#pragma once

namespace gb
{
    namespace cpu
    {
        class WordRegister
        {
        public:
            WordRegister()
            {}
            WordRegister(uint16_t value)
            {
                *this = value;
            }

            uint8_t& getHight()
            {
                return high_;
            }
            uint8_t& getLow()
            {
                return low_;
            }

            uint16_t value() const
            {
                return *this;
            }

            bool carried();
            bool halfCarried();

            WordRegister& operator+=(uint16_t value)
            {
                *this = *this + value;
                return *this;
            }
            WordRegister& operator-=(uint16_t value)
            {
                *this = *this - value;
                return *this;
            }
            WordRegister& operator=(uint16_t value)
            {
                low_ = value & 0x00FF;
                high_ = (value & 0xFF00) >> 8;
                return *this;
            }

            WordRegister operator--()
            {
                *this = *this - 1;
                return *this;
            }
            WordRegister operator++()
            {
                *this = *this + 1;
                return *this;
            }

            explicit operator bool()
            {
                return *this != 0;
            }

            operator uint16_t() const
            {
                return (high_ << 8) + low_;
            }
        private:
            uint8_t high_;
            uint8_t low_;
        };

        struct Registers
        {
            union 
            {
                uint16_t AF;
                
                struct {
                    union {
                        uint8_t value;

                        struct {
                            uint8_t unused : 4;
                            uint8_t C : 1;
                            uint8_t H : 1;
                            uint8_t N : 1;
                            uint8_t Z : 1;
                        };
                    }flags;

                    uint8_t A;
                };
            };

            // WordRegister BC;
            // uint8_t& C = BC.getLow();
            // uint8_t& B = BC.getHight();

            union {
                uint16_t BC;

                struct {
                    uint8_t C;
                    uint8_t B;
                };
            };

            // WordRegister DE;
            // uint8_t& E = DE.getLow();
            // uint8_t& D = DE.getHight();

            union {
                uint16_t DE;

                struct {
                    uint8_t E;
                    uint8_t D;
                };
            };

            // WordRegister HL;
            // uint8_t& L = HL.getLow();
            // uint8_t& H = HL.getHight();
            
            union {
                uint16_t HL;

                struct {
                    uint8_t L;
                    uint8_t H;
                };
            };

            // Stack pointer, program counter
            uint16_t SP;
            uint16_t PC;
        };
    }
}