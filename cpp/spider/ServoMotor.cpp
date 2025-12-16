#include "MMap.h"

#define DEGREE_MIN -90
#define DEGREE_MAX 90
#define SPEED_MAX 100
#define SPEED_MIN 0

// Use these definitions for _index_ arguments to the RegisterRead/RegisterWrite methods
#define PWM_PERIOD 0
#define PWM_DC 1
#define PWM_DELAY 2
#define PWM_READY 2
#define PWM_ABORT 3

// The DE10 clock frequency
#define FREQ 50000000
// The 20MS PWM period, in clock ticks
#define T_20MS (FREQ / 50)
// The maximum allowed PWM Duty cycle (corresponds to 90 degrees)
#define PWM_MAX (T_20MS * 2.5 / 20)
// The minimum allowed PWM Duty cycle (corresponds to -90 degrees)
#define PWM_MIN (T_20MS * 0.5 / 20)

#define DELAY_MIN 1000 // TODO replace this with your calculation from pre-lab 3
#define DELAY_MAX 2000 // TODO replace this with your calculation from pre-lab 3

class ServoMotor
{
public:
	//The current angle (in the range [-90,90]) of the servo motor.
	float m_fAngle;
	//The current effective operating speed.
	uint32_t m_speed;	

private:
	//The ID of the motor, this should correspond to the
	//PWM index (0-17) as specified in the Spider Robot documentation.
	//This is not needed by clients but is needed internally by the implementation.
	int m_nMotorID;
	// Stores a pointer to the memory map object that can communicate with the servo.
	MMap *_mmio;

    /**
	 * Given a speed value, s, convert it into an appropriate
	 * delay according to pre-lab part (3).
	 * @param s a speed value between SPEED_MIN and SPEED_MAX
	 * @return a delay in number of clock cycles
	 */
	int speedToDelay(int s)
	{
		float percent = s / 100;
		return DELAY_MAX - ((int)percent * (DELAY_MAX - DELAY_MIN));
	}

public:
	/**
	 * Save the given object for interfacing with MMIO and call the default contsructor.
	 * This method should also set the PWM signals to initial, default values using MMIO.
	 */
	ServoMotor(MMap *mmio, int motorId)
	{
		m_nMotorID = motorId;
		m_fAngle = 180.0;
		m_speed = 0;

		_mmio = mmio;
		// TODO use MMIO to set:
		// The PWM period
		// THE PWM duty cycle
		// The delay
		// Also set the Abort field to 0
		_mmio->Motor_Reg32_Write(m_nMotorID, PWM_PERIOD, T_20MS);
		_mmio->Motor_Reg32_Write(m_nMotorID, PWM_DC, 0);
		_mmio->Motor_Reg32_Write(m_nMotorID, PWM_DELAY, speedToDelay(50));
		_mmio->Motor_Reg32_Write(m_nMotorID, PWM_ABORT, 0);
	}

	
	/**
	 * Nothing to do on destruction.
	 */
	~ServoMotor(){}

    /**
	 * Calls the parent implementation to update the m_fangle member
	 * and makes sure it is in the bounds [-90, 90].
	 * Then, this must use MMIO to update the servo's duty cycle.
	 */
	void Move(float fAngle)
	{
		if (fAngle > DEGREE_MAX) {
			fAngle = DEGREE_MAX;
		} else if (fAngle < DEGREE_MIN) {
			fAngle = DEGREE_MIN;
		}
		m_fAngle = fAngle;
		// TODO compute the correct duty cycle from the current angle
		// and use MMIO to update the correct register
		int32_t dc = (uint32_t)(PWM_MIN + ((GetfAngle() - DEGREE_MIN) / (float)(DEGREE_MAX - DEGREE_MIN)) * (float)(PWM_MAX - PWM_MIN));
		_mmio->Motor_Reg32_Write(m_nMotorID, PWM_DC, dc);
	}


	/**
	 * Read from the Delay/Ready register using MMIO to
	 * determine if this servo has finished its last movement operation.
	 */
	bool IsReady()
	{
		return _mmio->Motor_Reg32_Read(m_nMotorID, PWM_READY);
	}

	/**
	 * See the documentation in Motor.h for this method.
	 * The parent implementation updates the m_speed member and makes
	 * sure that it is set in bounds [0, 100].
	 * Use the GetSpeed() method to access this value.
	 *
	 * This method must compute the delay based on the current
	 * speed and then must modifying the PWM registers using MMIO.
	 */
	void SetSpeed(int speed)
	{
		if (speed > SPEED_MAX) {
			speed = SPEED_MAX;
		} else if (speed < SPEED_MIN) {
			speed = SPEED_MIN;
		}
		m_speed = speed;
		// TODO update the PWM circuit registers using the appropriate MMIO address
		_mmio->Motor_Reg32_Write(m_nMotorID, PWM_DELAY, speedToDelay(GetSpeed()));
	}


	virtual float GetfAngle(){ return (m_fAngle == -0.0) ? 0.0f :  m_fAngle; }
  
	virtual uint32_t GetSpeed(){ return m_speed; }

	virtual void Reset(void) { 
		Move(0.0);
	}
};

