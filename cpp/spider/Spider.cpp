#include "SpiderLeg.cpp"
#include <unistd.h>

#define Knee_Up_Base 60
#define Knee_Down_Base 45
#define HipF_Base -20
#define HipM_Base 0
#define HipB_Base 20
#define Ankle_Base 45

class Spider {
	typedef enum{
		LEG_RF,
		LEG_RM,
		LEG_RB,
		LEG_LF,
		LEG_LM,
		LEG_LB,
		LEG_NUM
	} LEG_ID;

	typedef enum{
		TRIPOD1,    //RF LM RB
		TRIPOD2,	//LF RM LB
		TRIPOD_NUM
	} TRIPOD_ID;

	typedef enum{
		FWD,
		BACK
	} DIR;

	SpiderLeg *m_szLeg[LEG_NUM];
  
	TRIPOD_ID lastStep;
	DIR lastDir;
	MMap* _mmio;

	// Private helper method - declared here
	void MoveTripod(TRIPOD_ID Tripod, SpiderLeg::JOINT_ID Joint, float AngleF, float AngleM, float AngleB)
	{
		if (Tripod == 0){
			m_szLeg[LEG_RF]->MoveJoint(Joint, AngleF);
			m_szLeg[LEG_LM]->MoveJoint(Joint, AngleM);
			m_szLeg[LEG_RB]->MoveJoint(Joint, AngleB);
		}
		else{
			m_szLeg[LEG_LF]->MoveJoint(Joint, AngleF);
			m_szLeg[LEG_RM]->MoveJoint(Joint, AngleM);
			m_szLeg[LEG_LB]->MoveJoint(Joint, AngleB);
		}
	}

public:

	Spider()
	{
		_mmio = new MMap();		
		int szMotorID[] = {
			/* LEG_RF */ 0, 1, 2,
			/* LEG_RM */ 3, 4, 5,
			/* LEG_RB */ 6, 7, 8,
			/* LEG_LF */ 9, 10, 11,
			/* LEG_LM */ 12, 13, 14,
			/* LEG_LB */ 15, 16, 17};

		for (int i = 0; i < LEG_NUM; i++)
		{
			// Reverse the angles on all of the RHS motors
			m_szLeg[i] = new SpiderLeg(_mmio, szMotorID[i * 3], szMotorID[i * 3 + 1], szMotorID[i * 3 + 2], i == LEG_RF || i == LEG_RB || i == LEG_RM);
		}
		lastStep = TRIPOD2;
		lastDir = FWD;
	}

	~Spider() { for (int i = 0; i < LEG_NUM; i++)  delete m_szLeg[i]; }

	void Init()
	{
		// Init -- Set all servos to neutral position (0 degrees) first
		// Do this for all legs simultaneously to avoid imbalance
		for (int i = 0; i < LEG_NUM; i++){
			m_szLeg[i]->MoveJoint(SpiderLeg::Hip, 0.0);
			m_szLeg[i]->MoveJoint(SpiderLeg::Knee, 0.0);
			m_szLeg[i]->MoveJoint(SpiderLeg::Ankle, 0.0);
		}
		WaitReady();
		
		// Small delay to ensure all servos have initialized
		usleep(500000); // 0.5 second delay
	}

	bool WaitReady()
	{
		bool bReady = false;
		while (!bReady) bReady = IsReady();
		return bReady;
	}

	bool IsReady()
	{
		bool bReady = true;
		for (int i = 0; i < LEG_NUM && bReady; i++)
			if (!m_szLeg[i]->IsReady()) bReady = false;
		return bReady;
	}

	void MoveForward()
	{
		if ((lastStep == TRIPOD2 && lastDir == FWD) || (lastStep == TRIPOD1 && lastDir == BACK)){
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20);
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD1;
		}else{
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20);
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20);
			WaitReady();
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD2;
		}
		lastDir = FWD;
	}

	void MoveBackward()
	{
		if ((lastStep == TRIPOD2 && lastDir == BACK) || (lastStep == TRIPOD1 && lastDir == FWD)){
			// Lift TRIPOD1 legs
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			// Move hips in opposite direction from forward (notice the sign flip)
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20);
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20);
			WaitReady();
			// Lower TRIPOD1 legs
			MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD1;
		} else {
			// Lift TRIPOD2 legs
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
			WaitReady();
			// Move hips in opposite direction from forward (notice the sign flip)
			MoveTripod(TRIPOD1, SpiderLeg::Hip, HipF_Base + 20, HipM_Base + 20, HipB_Base + 20);
			MoveTripod(TRIPOD2, SpiderLeg::Hip, HipF_Base - 20, HipM_Base - 20, HipB_Base - 20);
			WaitReady();
			// Lower TRIPOD2 legs
			MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
			WaitReady();
			lastStep = TRIPOD2;
		}
		lastDir = BACK;
	}

void LeftTurn()
{
    // Complete Left Turn (CCW) - body rotates counterclockwise
    
    // Phase 1 & 2: Raise Tripod B and rotate CCW
    MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
    WaitReady();
    
    // Tripod B rotates CCW: right side forward, left side backward
    m_szLeg[LEG_RF]->MoveJoint(SpiderLeg::Hip, HipF_Base + 20);  // Right front forward
    m_szLeg[LEG_LM]->MoveJoint(SpiderLeg::Hip, HipM_Base - 20);  // Left middle backward
    m_szLeg[LEG_RB]->MoveJoint(SpiderLeg::Hip, HipB_Base + 20);  // Right back forward
    WaitReady();
    
    // Phase 3: Lower Tripod B
    MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
    WaitReady();
    
    // Phase 4: Raise Tripod R
    MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
    WaitReady();
    
    // Phase 5: Tripod B ALL legs rotate CW (all go back to base position)
    // This is CW rotation of the grounded tripod that pushes body CCW
    m_szLeg[LEG_RF]->MoveJoint(SpiderLeg::Hip, HipF_Base);
    m_szLeg[LEG_LM]->MoveJoint(SpiderLeg::Hip, HipM_Base);
    m_szLeg[LEG_RB]->MoveJoint(SpiderLeg::Hip, HipB_Base);
    WaitReady();
    
    // Phase 6: Lower Tripod R
    MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
    WaitReady();
    
    lastStep = TRIPOD2;
    lastDir = FWD;
}

void RightTurn()
{
    // Complete Right Turn (CW) - body rotates clockwise
    
    // Phase 1 & 2: Raise Tripod B and rotate CW
    MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
    WaitReady();
    
    // Tripod B rotates CW: right side backward, left side forward
    m_szLeg[LEG_RF]->MoveJoint(SpiderLeg::Hip, HipF_Base - 20);  // Right front backward
    m_szLeg[LEG_LM]->MoveJoint(SpiderLeg::Hip, HipM_Base + 20);  // Left middle forward
    m_szLeg[LEG_RB]->MoveJoint(SpiderLeg::Hip, HipB_Base - 20);  // Right back backward
    WaitReady();
    
    // Phase 3: Lower Tripod B
    MoveTripod(TRIPOD1, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
    WaitReady();
    
    // Phase 4: Raise Tripod R
    MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Up_Base, Knee_Up_Base, Knee_Up_Base);
    WaitReady();
    
    // Phase 5: Tripod B ALL legs rotate CCW (all go back to base position)
    // This is CCW rotation of the grounded tripod that pushes body CW
    m_szLeg[LEG_RF]->MoveJoint(SpiderLeg::Hip, HipF_Base);
    m_szLeg[LEG_LM]->MoveJoint(SpiderLeg::Hip, HipM_Base);
    m_szLeg[LEG_RB]->MoveJoint(SpiderLeg::Hip, HipB_Base);
    WaitReady();
    
    // Phase 6: Lower Tripod R
    MoveTripod(TRIPOD2, SpiderLeg::Knee, Knee_Down_Base, Knee_Down_Base, Knee_Down_Base);
    WaitReady();
    
    lastStep = TRIPOD2;
    lastDir = FWD;
}

	void Standup()
	{
		bool bSuccess;

		// Step 1: Position hips to base angles
		float fszJoin0Angle[] = {HipF_Base, HipM_Base, HipB_Base,
								HipF_Base, HipM_Base, HipB_Base};
		for (int i = 0; i < LEG_NUM; i++)
			m_szLeg[i]->MoveJoint(SpiderLeg::Hip, fszJoin0Angle[i]);
		
		bSuccess = WaitReady();

		// Step 2: Extend legs horizontally first (Knee at 90, Ankle at 45)
		for (int i = 0; i < LEG_NUM; i++)
		{
			m_szLeg[i]->MoveJoint(SpiderLeg::Knee, 90);
			m_szLeg[i]->MoveJoint(SpiderLeg::Ankle, 45);
		}
		bSuccess = WaitReady();
		
		// Step 3: Gradually lower the body by bending knees
		// Move slowly from 90 degrees down to 45 degrees
		float KneeAngle = 90;
		const float AnkleAngle = 45.0;
		while (bSuccess && KneeAngle >= Knee_Down_Base)
		{
			for (int i = 0; i < LEG_NUM; i++)
			{
				m_szLeg[i]->MoveJoint(SpiderLeg::Knee, KneeAngle);
				m_szLeg[i]->MoveJoint(SpiderLeg::Ankle, AnkleAngle);
			} 
			bSuccess = WaitReady();
			KneeAngle -= 5.0;
		}
		
		// Step 4: Test each leg by lifting and lowering one at a time
		if (bSuccess) {
			std::cout << "Testing legs..." << std::endl;
			
			for (int i = 0; i < LEG_NUM; i++)
			{
				// Lift leg
				m_szLeg[i]->MoveJoint(SpiderLeg::Knee, Knee_Up_Base);
				WaitReady();
				
				// Lower leg
				m_szLeg[i]->MoveJoint(SpiderLeg::Knee, Knee_Down_Base);
				WaitReady();
			}
			
			std::cout << "Leg test complete!" << std::endl;
		}
		
		// Step 5: Final reset to ensure all legs are in proper standing position
		if (bSuccess) Reset();
	}

	void Reset()
	{
		float fszJoin0Angle[] = {HipF_Base, HipM_Base, HipB_Base,
								 HipF_Base, HipM_Base, HipB_Base};

		////////////////////////
		////Reset Hip Knee ankle
		for (int i = 0; i < LEG_NUM - 3; i++)
		{
			m_szLeg[i]->MoveJoint(SpiderLeg::Knee, Knee_Up_Base);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Knee, Knee_Up_Base);
			m_szLeg[i]->MoveJoint(SpiderLeg::Hip, fszJoin0Angle[i]);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Hip, fszJoin0Angle[LEG_NUM - i - 1]);
			m_szLeg[i]->MoveJoint(SpiderLeg::Ankle, Ankle_Base);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Ankle, Ankle_Base);
			WaitReady();
			m_szLeg[i]->MoveJoint(SpiderLeg::Knee, Knee_Down_Base);
			m_szLeg[LEG_NUM - i - 1]->MoveJoint(SpiderLeg::Knee, Knee_Down_Base);
			WaitReady();
		}
	}

};