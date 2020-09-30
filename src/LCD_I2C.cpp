/*
    LCD_I2C - Arduino library to control a 16x2 LCD via an I2C adapter based on PCF8574

    Copyright(C) 2020 Blackhack <davidaristi.0504@gmail.com>

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#include "LCD_I2C.h"
#include "Wire.h"

void LCD_I2C::begin(bool beginWire)
{
    if (beginWire)
        Wire.begin();

    I2C_Write(0b00000000); // Clear i2c adapter
    delay(50); //Wait more than 40ms after powerOn.

    InitializeLCD();
}

void LCD_I2C::backlight(bool on)
{
    _output.Led = on ? 1 : 0;
    I2C_Write(0b00000000 | _output.Led << 3); // Led pin is independent from LCD data and control lines.
}

void LCD_I2C::clear()
{
    _output.rs = 0;
    _output.rw = 0;

    LCD_Write(0b00000001);
    delayMicroseconds(1600);
}

void LCD_I2C::home()
{
    _output.rs = 0;
    _output.rw = 0;

    LCD_Write(0b00000010);
    delayMicroseconds(1600);
}

// Part of Entry mode set
void LCD_I2C::leftToRight()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState |= 1 << 1;

    LCD_Write(0b00000100 | _entryState);
    delayMicroseconds(37);
}

// Part of Entry mode set
void LCD_I2C::rightToLeft()
{
    _output.rs = 0;
    _output.rw = 0;

    _entryState &= ~(1 << 1);

    LCD_Write(0b00000100 | _entryState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C::display()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1 << 2;

    LCD_Write(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C::noDisplay()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~(1 << 2);

    LCD_Write(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C::cursor()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1 << 1;

    LCD_Write(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C::noCursor()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~(1 << 1);

    LCD_Write(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C::blink()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState |= 1;

    LCD_Write(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Part of Display control
void LCD_I2C::noBlink()
{
    _output.rs = 0;
    _output.rw = 0;

    _displayState &= ~1;

    LCD_Write(0b00001000 | _displayState);
    delayMicroseconds(37);
}

// Set DDRAM address
void LCD_I2C::setCursor(uint8_t row, uint8_t col)
{
    _output.rs = 0;
    _output.rw = 0;

    uint8_t newAddress = (row == 0 ? 0x00 : 0x40);
    newAddress += col;

    LCD_Write(0b10000000 | newAddress);
    delayMicroseconds(37);
}

size_t LCD_I2C::write(uint8_t character)
{
    _output.rs = 1;
    _output.rw = 0;

    LCD_Write(character);
    delayMicroseconds(41);
}

void LCD_I2C::InitializeLCD()
{
    // See HD44780U datasheet "Initializing by Instruction" Figure 24 (4-Bit Interface)
    _output.rs = 0;
    _output.rw = 0;

    LCD_Write(0b00110000, true);
    delayMicroseconds(4200);
    LCD_Write(0b00110000, true);
    delayMicroseconds(150);
    LCD_Write(0b00110000, true);
    delayMicroseconds(37);
    LCD_Write(0b00100000, true); // Function Set - 4 bits mode
    delayMicroseconds(37);
    LCD_Write(0b00101000); // Function Set - 4 bits(Still), 2 lines, 5x8 font
    delayMicroseconds(37);

    display();
    clear();
    leftToRight();
}

void LCD_I2C::I2C_Write(uint8_t output)
{
    Wire.beginTransmission(_address);
    Wire.write(output);
    Wire.endTransmission();
}

void LCD_I2C::LCD_Write(uint8_t output, bool initialization)
{
    _output.data = output;

    _output.E = true;
    I2C_Write(_output.GetHighData());
    delayMicroseconds(1); // High part of enable should be >450 nS

    _output.E = false;
    I2C_Write(_output.GetHighData());

    // During initialization we only send half a byte
    if (!initialization)
    {
        delayMicroseconds(37); // I think we need a delay between half byte writes, but no sure how long it needs to be.

        _output.E = true;
        I2C_Write(_output.GetLowData());
        delayMicroseconds(1); // High part of enable should be >450 nS

        _output.E = false;
        I2C_Write(_output.GetLowData());
    }
    //delayMicroseconds(37); // Some commands have different timing requirement,
                             // so every command should handle its own delay after execution
}