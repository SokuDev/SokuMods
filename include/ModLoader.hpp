//
// Created by PinkySmile on 05/08/2022.
//

#ifndef SWRSTOYS_MODLOADER_HPP
#define SWRSTOYS_MODLOADER_HPP

#include <string>

namespace SokuModLoader
{
	class IValue {
	protected:
		char *_key;

	public:
		IValue(const char *key) : _key(strdup(key)) {};
		virtual ~IValue() { free(this->_key); };
		virtual bool setString(const char *str) = 0;
		virtual const char *getString() const = 0;
		virtual const char *getName() const = 0;
	};

	class IniValue : public IValue {
	protected:
		char *_section;
		char *_path;

	public:
		IniValue(const char *section, const char *key, const char *path) :
			IValue(key),
			_section(strdup(section)),
			_path(strdup(path))
		{
		}

		~IniValue() override
		{
			free(this->_section);
			free(this->_path);
		}

		const char *getName() const override
		{
			return this->_key;
		}

		virtual void commitChanges() const
		{
			WritePrivateProfileString(this->_section, this->_key, this->getString(), this->_path);
		}
	};

	class IniIntValue : public IniValue {
	private:
		int _value = 0;
		char _buffer[12];

	public:
		IniIntValue(const char *section, const char *key, int defaultValue, const char *path) :
			IniValue(section, key, path)
		{
			this->_value = GetPrivateProfileInt(this->_section, this->_key, defaultValue, this->_path);
			sprintf(this->_buffer, "%i", this->_value);
		}

		bool setString(const char *str) override
		{
			char *ptr;
			long value = strtol(str, &ptr, 10);

			if (value < INT_MIN || value > INT_MAX)
				return false;
			if (*ptr)
				return false;
			this->_value = value;
			sprintf(this->_buffer, "%i", this->_value);
			return true;
		}

		virtual const char *getString() const
		{
			return this->_buffer;
		}

		int getValue() const
		{
			return this->_value;
		}

		void setValue(int value)
		{
			this->_value = value;
			sprintf(this->_buffer, "%i", this->_value);
		}
	};

	class IniStringValue : public IniValue {
	private:
		char *_value;
		size_t _maxSize;

	public:
		IniStringValue(const char *section, const char *key, const char *defaultValue, size_t maxSize, const char *path) :
			IniValue(section, key, path),
			_value((char*)malloc(maxSize + 1)),
			_maxSize(maxSize)
		{
			if (!this->_value)
				throw std::bad_alloc();
			GetPrivateProfileString(this->_section, this->_key, defaultValue, this->_value, maxSize, this->_path);
		}

		bool setString(const char *str) override
		{
			if (strlen(str) >= this->_maxSize)
				return false;
			strcpy(this->_value, str);
			return true;
		}

		virtual const char *getString() const
		{
			return this->_value;
		}
	};
}

#endif //SWRSTOYS_MODLOADER_HPP
