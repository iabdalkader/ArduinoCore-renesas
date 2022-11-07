#include "Arduino.h"
#include "FspTimer.h"

void tone_timer_callback(timer_callback_args_t *args);

class Tone {
    uint32_t           frequency;
    uint32_t           duration;
    uint32_t           limit;
    pin_size_t         pin;
    uint8_t            status = LOW;
    static FspTimer    tone_timer;
    static int         channel;

public:
    Tone(pin_size_t pin, unsigned int frequency, unsigned long duration) : frequency(frequency), duration(duration), pin(pin)  {
        pinMode(pin, OUTPUT);
        if (frequency) {
            timer_config(500000 / frequency);
        }
    }

    ~Tone() {
        stop();
    }

    void start(void) {
        if (frequency) {
            tone_timer.start();
        }
        if (duration != 0) {
            limit = millis() + duration;
        }
    }

    void toggle() {
    	digitalWrite(pin, status);
        status = !!!status;
        if (millis() > limit) {
            stop();
        }
    }

    void stop(void) {
        if (frequency) {
            tone_timer.stop();
        }
        digitalWrite(pin, LOW);
    }

    static int timer_config(uint32_t period_us) {
        // Configure and enable the tone timer.
        uint8_t type = 0;
        if (channel == -1) {
            channel = FspTimer::get_available_timer(type);
        }

        tone_timer.begin(TIMER_MODE_PERIODIC, type, channel,
                1000000.0f/period_us, 50.0f, tone_timer_callback, nullptr);
        tone_timer.setup_overflow_irq();
        tone_timer.open();
        tone_timer.stop();
    }
};

FspTimer Tone::tone_timer;
int Tone::channel = -1;
static Tone* active_tone = NULL;

void tone_timer_callback(timer_callback_args_t *args) {
    active_tone->toggle();
}

void tone(pin_size_t pin, unsigned int frequency, unsigned long duration) {
	if (active_tone) {
		delete active_tone;
	}
	Tone* t = new Tone(pin, frequency, duration);
	t->start();
	active_tone = t;
};

void noTone(pin_size_t pin) {
	if (active_tone) {
		active_tone->stop();
		delete active_tone;
		active_tone = NULL;
	}
};
