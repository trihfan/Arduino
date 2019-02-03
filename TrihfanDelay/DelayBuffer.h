#include <bitset>

template <size_t buffer_max_size>
class DelayBuffer
{
public:
    /*
     * Read a 12 bits integer
     */
    uint16_t read(size_t index) const
    {
        uint16_t value = 0;
        index = index * 12;

        for (size_t i = 0; i < 12; i++)
        {
            value += buffer[index + i] << i;
        }
        return value;
    }

    /*
     * Write a 12 bits integer
     */
    void write(size_t index, uint16_t value)
    {
        index = index * 12;

        for (size_t i = 0; i < 12; i++)
        {
            buffer[index + i] = value & (1 << i);
        }
    }

private:
    /*
    * The bitset buffer
    */
    std::bitset<buffer_max_size> buffer { 0 };
};