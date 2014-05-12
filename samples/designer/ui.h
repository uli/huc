#define RES_X 512

/* Envelope */
#define ENV_START 1
#define ENV_X 8
#define ENV_X_RIGHT (ENV_X + ENV_STEP_X * 16)
#define ENV_Y 168
#define ENV_Y_TOP (ENV_Y - ENV_STEP_Y * 32)
#define ENV_STEP_X 16
#define ENV_STEP_Y 2
#define ENV_CENTER 8

/* Waveform */
#define WAVE_START (ENV_START + 16)
#define WAVE_X 8
#define WAVE_X_RIGHT (WAVE_X + WAVE_STEP_X * 32)
#define WAVE_Y 80
#define WAVE_Y_TOP (WAVE_Y - WAVE_STEP_Y * 32)
#define WAVE_STEP_X 12
#define WAVE_STEP_Y 2
#define WAVE_CENTER 8

/* Piano keys */
#define PIANO_X (RES_X/2/8-26)
#define PIANO_Y 24

/* Noise checkbox */
#define NOISE_X 53
#define NOISE_Y 2

/* Transposition */
#define OCTAVE_NUM_X 55
#define OCTAVE_NUM_Y 5
#define OCTAVE_PLUS_X 59
#define OCTAVE_PLUS_Y 5
#define OCTAVE_MINUS_X 53
#define OCTAVE_MINUS_Y 5

/* Standard waveform selection */
#define STDWAVE_NUM_X 55
#define STDWAVE_NUM_Y 8
#define STDWAVE_PLUS_X 59
#define STDWAVE_PLUS_Y 8
#define STDWAVE_MINUS_X 53
#define STDWAVE_MINUS_Y 8

