#ifndef __I2C_LIB_H
#define __I2C_LIB_H

int  i2cOpen(char *device);
void i2cClose();
int  i2cSetAddress(int address);
int  writeRegByte(uint8_t reg, uint8_t *value);
int  writeRegWord(uint8_t reg, uint16_t *value);
int  readRegByte(uint8_t reg, uint8_t *value);
int  readRegWord(uint8_t reg, uint16_t *value);
int  readRegBytes(uint8_t reg, uint8_t *buf, int len);
int  setRegBitState(uint8_t reg, uint8_t state, uint8_t bitMask);

#endif //__I2C_LIB_H
