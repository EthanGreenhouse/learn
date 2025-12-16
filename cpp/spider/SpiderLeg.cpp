#include "ServoMotor.cpp"

class SpiderLeg {
public:
	typedef enum{
		Hip=0,
		Knee,
		Ankle,
		JOINT_NUM
	} JOINT_ID;

private:
	ServoMotor *m_szMotor[JOINT_NUM];
	bool m_reverse;

public:
	//Use reverse to flip the angle interpretation
	SpiderLeg(MMap* m, int Joint0_MotorID,int Joint1_MotorID,int Joint2_MotorID, bool reverse)
	{
		m_reverse = reverse;
		m_szMotor[0] = new ServoMotor(m, Joint0_MotorID);
		m_szMotor[1] = new ServoMotor(m, Joint1_MotorID);
		m_szMotor[2] = new ServoMotor(m, Joint2_MotorID);
	}
	
	~SpiderLeg() { for(int i=0;i<JOINT_NUM;i++)  delete m_szMotor[i]; }

	void Reset(void) { for(int i=0;i<JOINT_NUM;i++)  m_szMotor[i]->Reset(); }

	void MoveJoint(JOINT_ID JointID, float fAngle) {
		m_szMotor[JointID]->Move((m_reverse) ? -fAngle : fAngle);
	}

	bool IsReady(void){
		bool bReady = true;
		for(int i=0;i<JOINT_NUM && bReady;i++){
			if (!m_szMotor[i]->IsReady())
				bReady = false;
		}
		return bReady;
	}

	float GetfAngle(JOINT_ID JointID)
	{
		float tmp = m_szMotor[JointID]->GetfAngle();
		return (m_reverse) ? -tmp : tmp;
	}

};
