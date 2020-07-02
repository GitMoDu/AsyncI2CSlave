// TemplateI2CDriver.h

#ifndef _TEMPLATE_I2C_DRIVER_h
#define _TEMPLATE_I2C_DRIVER_h

#include <Arduino.h>
#include <I2CSlaveBaseAPI.h>
#include <TemplateMessageI2C.h>
#include <Wire.h>

template <const uint8_t DeviceAddress,
	const uint32_t DeviceId,
	const uint32_t MillisBeforeResponse = 20>
	class TemplateI2CDriver
{
private:
	static const uint8_t SetupRetryMaxCount = 3;
	TwoWire* I2CInstance = nullptr;

protected:
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> OutgoingMessage;
	TemplateVariableMessageI2C<BaseAPI::MessageMaxSize> IncomingMessage;

public:
	uint32_t GetDeviceId() { return DeviceId; }

public:
	TemplateI2CDriver(TwoWire* i2cInstance)
	{
		I2CInstance = i2cInstance;
	}

	virtual bool Setup()
	{
		if (DeviceAddress <= I2C_ADDRESS_MIN_VALUE ||
			DeviceAddress > I2C_ADDRESS_MAX_VALUE)
		{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
			Serial.println(F("Invalid I2C Address."));
#endif
			return false;
		}

		if (I2CInstance == nullptr)
		{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
			Serial.println(F("Device Not Setup."));
#endif
			return false;
		}

		for (uint8_t i = 0; i < SetupRetryMaxCount; i++)
		{
			if (CheckDevice())
			{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
				Serial.println(F("Device detected."));
#endif
				return true;
			}
			else
			{
#ifdef DEBUG_TEMPLATE_I2C_DRIVER
				Serial.println(F("Device Not detected."));
#endif
			}
		}
		return false;
	}

	virtual bool CheckDevice()
	{
#ifdef I2C_DRIVER_MOCK_I2C
		return true;
#else
		bool Ok = SendMessageHeader(BaseAPI::GetDeviceId.Header);

#ifdef I2C_SLAVE_DEVICE_ID_ENABLE
		delay(MillisBeforeResponse);

		if (Ok)
		{
			if (!GetResponse(BaseAPI::GetDeviceId.ResponseLength)
				|| (IncomingMessage.Get32Bit(0) != GetDeviceId()))
			{
				Ok = false;
			}
		}
#else

		return Ok;
#endif
#endif
	}

	bool GetResponse(const uint8_t requestSize)
	{
		IncomingMessage.Clear();

		I2CInstance->requestFrom(DeviceAddress, requestSize);

		while (I2CInstance->available())
		{
			IncomingMessage.FastWrite(I2CInstance->read());
		}

		return IncomingMessage.Length == requestSize;
	}

protected:
	bool WriteCurrentMessage()
	{
#ifndef I2C_DRIVER_MOCK_I2C
		I2CInstance->beginTransmission(DeviceAddress);
		I2CInstance->write(OutgoingMessage.Data, OutgoingMessage.Length);

		return I2CInstance->endTransmission() == 0;
#else
		return true;
#endif		
	}

	// Quick message senders.
	bool SendMessageHeader(const uint8_t header)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Length = 1;

		return WriteCurrentMessage();
	}

	bool SendMessageSingle16(const uint8_t header, uint16_t value)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Set16Bit(value, 1);
		OutgoingMessage.Length = 3;

		return WriteCurrentMessage();
	}

	bool SendMessageSingle32(const uint8_t header, uint32_t value)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Set32Bit(value, 1);
		OutgoingMessage.Length = 5;

		return WriteCurrentMessage();
	}

	bool SendMessageDual16(const uint8_t header, uint16_t value1, uint16_t value2)
	{
		OutgoingMessage.Clear();
		OutgoingMessage.SetHeader(header);
		OutgoingMessage.Set16Bit(value1, 1);
		OutgoingMessage.Set16Bit(value2, 3);
		OutgoingMessage.Length = 5;

		return WriteCurrentMessage();
	}
};


#endif
