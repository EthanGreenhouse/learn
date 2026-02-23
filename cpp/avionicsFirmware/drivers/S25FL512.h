#ifndef S25FL512_H
#define S25FL512_H

#include "Arduino.h"

class FlashMemory {
public:
    virtual ~FlashMemory() = default;

    virtual void setup() {}

    virtual bool ready() const = 0;

    virtual void waitForReady(uint32_t timeout) const  = 0;

    virtual void write(uint32_t address, const uint8_t *buffer, uint32_t length) const  = 0;

    virtual void read(uint32_t address, uint8_t *buffer, uint32_t length) const  = 0;

    virtual void eraseAll() const = 0;

    virtual uint32_t getMemorySizeBytes() const = 0;
};


class S25FL512 final : public FlashMemory {
public:
    explicit S25FL512(uint8_t chipSelectPin);

    void setup() override;

    uint32_t getMemorySizeBytes() const override;

    bool ready() const override;

    void waitForReady(uint32_t timeout) const override;

    void write(uint32_t address, const uint8_t* buffer, uint32_t length) const override;

    void read(uint32_t address, uint8_t* buffer, uint32_t length) const override;

    void eraseAll() const override;

private:
    void pageProgram(uint32_t address, const uint8_t* buffer, uint32_t length) const;

    void enableWrite() const;

    void disableWrite() const;


    uint8_t m_chipSelectPin;
};

#endif
