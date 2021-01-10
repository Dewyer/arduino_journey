#include <Arduino.h>

class LedGame
{
public:
	LedGame(size_t led_count, int *led_pins, int led_steps, int btn_pin);

	void setup();

	void loop();

private:
	size_t _led_count;
	int *_led_pins;
	int _led_steps;
	int _at_step;
	int _on_led;
	int _btn_pin;
	bool _done;

	void on_lose();
	void on_win();
};

LedGame::LedGame(size_t led_count, int *led_pins, int led_steps, int btn_pin)
{
	this->_led_count = led_count;
	this->_led_pins = led_pins;
	this->_led_steps = led_steps;
	this->_on_led = 0;
	this->_at_step = 0;
	this->_btn_pin = btn_pin;
	this->_done = false;
}

void LedGame::setup()
{
	for (int ii = 0; ii < this->_led_count; ++ii) {
		pinMode(this->_led_pins[ii], OUTPUT);
		delay(10);
		digitalWrite(this->_led_pins[ii], LOW);
	}

	pinMode(this->_btn_pin, INPUT);
}

void LedGame::on_lose()
{
	for (int ii = 0; ii < this->_led_count; ++ii) {
		digitalWrite(this->_led_pins[ii], HIGH);
	}
	Serial.println("You lost!");
}

void LedGame::on_win()
{
	Serial.println("You won!");
}

void LedGame::loop()
{
	if (this->_led_count == 0 || this->_done)
		return;

	this->_at_step = (this->_at_step + 1) % this->_led_steps;
	if (this->_at_step == 0) {
		digitalWrite(this->_led_pins[this->_on_led], LOW);
		this->_on_led = (this->_on_led + 1) % this->_led_count;
		digitalWrite(this->_led_pins[this->_on_led], HIGH);
	}

	int btn_pressed = digitalRead(this->_btn_pin);

	if (btn_pressed == HIGH)
	{
		if (this->_on_led == this->_led_count/2)
			this->on_win();
		else
			this->on_lose();

		this->_done = true;
	}

	delay(5);
}

int led_game_pins[11] = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3};
LedGame led_game_inst(11, (int*)&led_game_pins, 15, 2);

void setup()
{
	Serial.begin(9600);
	led_game_inst.setup();
}

void loop()
{
	led_game_inst.loop();
}
