#define latchPin 11
#define clockPin 10
#define dataPin 9

class ShiftRegister
{
public:
	ShiftRegister(int latch, int clock, int data);
	void setup();
	void set_byte(byte val);
private:
	int latch;
	int clock;
	int data;
};

ShiftRegister::ShiftRegister(int latch, int clock, int data)
{
	this->latch = latch;
	this->clock = clock;
	this->data = data;
}

void ShiftRegister::setup()
{
	pinMode(this->latch, OUTPUT);
	pinMode(this->clock, OUTPUT);
	pinMode(this->data, OUTPUT);
}

void ShiftRegister::set_byte(byte val)
{
	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, LSBFIRST, val);
	digitalWrite(latchPin, HIGH);
}

class FourDigits{
public:
	FourDigits(int *digit_pins, ShiftRegister *seg_reg);
	void setup();
	void enable(bool on);
	void set_number(int nn);
	void loop();
private:
	int number;
	int at_digit;
	int *digit_pins;
	bool enabled;
	ShiftRegister *seg_reg;
};

FourDigits::FourDigits(int * digit_pins, ShiftRegister * seg_reg)
{
	this->digit_pins = digit_pins;
	this->seg_reg = seg_reg;
	this->at_digit = 0;
	this->number = -1;
	this->enabled = true;
}

void FourDigits::setup()
{
	this->seg_reg->setup();

	for (int ii = 0; ii < 4; ++ii) {
		pinMode(this->digit_pins[ii], OUTPUT);
	}
}

void FourDigits::enable(bool on)
{
	this->enabled = on;
}

void FourDigits::set_number(int nn)
{
	if (nn < 9999 && nn > 0)
		this->number = nn;
}

byte digit_to_segmented_byte(int dig)
{
	//PGFEDCBA
	//ABCDEFGP
	switch (dig) {
		case 0:
			return 3;
		case 1:
			return 159;
		case 2:
			return 37;
		case 3:
			return 13;
		case 4:
			return 153;
		case 5:
			return 73;
		case 6:
			return 65;
		case 7:
			return 31;
		case 8:
			return 1;
		case 9:
			return 9;
		default:
			return 0;
	}
}

int pow_ten(int exp)
{
	int kk = 1;
	for (int ii = 0; ii < exp; ++ii) {
		kk*=10;
	}
	return kk;
}

void FourDigits::loop()
{
	if (!this->enabled){
		for (int ii = 0; ii < 4; ++ii) {
			digitalWrite(this->digit_pins[ii], LOW);
		}
		return;
	}

	this->at_digit = (this->at_digit + 1) % 4;
	int dig_to_w = this->number / pow_ten(this->at_digit) % 10;

	/*
	Serial.print(this->number);
	Serial.print(" - ");
	Serial.print(this->at_digit);
	Serial.print(" at dig, ");
	Serial.print(pow_ten(this->at_digit));
	Serial.println(" digit");
	*/

	for (int ii = 0; ii < 4; ++ii) {
		digitalWrite(this->digit_pins[ii], ii == this->at_digit ? HIGH : LOW);
	}
	this->seg_reg->set_byte(digit_to_segmented_byte(dig_to_w));
}

enum GuessGameLastOutCome {Lower, Higher, Over, None};

class GuessGame{
public:
	GuessGame(int pot_pin, int btn_pin, int red_pin, int green_pin, FourDigits *digits, int led_time, int tolerance);
	void setup();
	void loop();
private:
	int pot_pin;
	int btn_pin;
	int red_pin;
	int green_pin;
	int guessed_num;
	GuessGameLastOutCome last_outcome;
	int outcome_steps;
	int led_time;
	int tolerance;
	FourDigits *digits;

	void handle_output_leds();
};

GuessGame::GuessGame(int pot_pin, int btn_pin, int red_pin, int green_pin, FourDigits *digits, int led_time, int tolerance)
{
	this->pot_pin = pot_pin;
	this->btn_pin = btn_pin;
	this->red_pin = red_pin;
	this->green_pin = green_pin;
	this->digits = digits;
	this->last_outcome = GuessGameLastOutCome::None;
	this->outcome_steps = 0;
	this->led_time = led_time;
	this->tolerance = tolerance;

	this->guessed_num = 420;
}

int decrement_to_zero(int nn)
{
	if (nn != 0)
		return nn-1;
	return nn;
}

void GuessGame::handle_output_leds()
{
	bool g_on = false;
	bool r_on = false;
	this->outcome_steps = decrement_to_zero(this->outcome_steps);

	if (this->last_outcome == GuessGameLastOutCome::Over)
	{
		g_on = true;
		r_on = true;
	} else if (this->last_outcome == GuessGameLastOutCome::Higher && this->outcome_steps)
	{
		g_on = true;
	} else if (this->last_outcome == GuessGameLastOutCome::Lower && this->outcome_steps)
	{
		r_on = true;
	}

	digitalWrite(this->green_pin, g_on ? HIGH : LOW);
	digitalWrite(this->red_pin, r_on ? HIGH : LOW);
}

void GuessGame::setup()
{
	this->digits->setup();
	pinMode(this->green_pin, OUTPUT);
	pinMode(this->red_pin, OUTPUT);
	pinMode(this->btn_pin, INPUT);
	pinMode(this->pot_pin, INPUT);
}

bool within_tolerance(int nn, int target, int tolerance)
{
	return abs(nn-target) <= tolerance;
}

void GuessGame::loop()
{
	if (this->last_outcome == GuessGameLastOutCome::Over)
	{
		this->handle_output_leds();
		this->digits->set_number(this->guessed_num);
		this->digits->loop();
		return;
	}

	int pot_val = analogRead(this->pot_pin);
	int btn_pressed = digitalRead(this->btn_pin);

	Serial.println(pot_val);

	if (btn_pressed == HIGH)
	{
		Serial.println("Guess");
		if (within_tolerance(pot_val, this->guessed_num, this->tolerance))
			this->last_outcome = GuessGameLastOutCome::Over;
		else if (pot_val > this->guessed_num)
			this->last_outcome = GuessGameLastOutCome::Lower;
		else
			this->last_outcome = GuessGameLastOutCome::Higher;

		this->outcome_steps = this->led_time;
	}
	this->handle_output_leds();
	this->digits->set_number(pot_val);
	this->digits->loop();
}

int digit_pins[] = {4,5,6,7};
ShiftRegister segment_reg(latchPin, clockPin, dataPin);
FourDigits four_digits((int*)&digit_pins, &segment_reg);
GuessGame g_game(1, 12, 3, 2, &four_digits, 20, 10);

void setup()
{
	Serial.begin(9600);
	g_game.setup();
}

void loop()
{
	g_game.loop();
	delay(50);
}


