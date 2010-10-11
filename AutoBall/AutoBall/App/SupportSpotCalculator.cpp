#include "Stdafx.h"
#include "Entity/PlayerBase.h"
#include "Goal.h"
#include "Entity/SoccerBall.h"
#include "SoccerTeam.h"
#include "SoccerPitch.h"
#include "../Public/GameSetup.h"
#include "SupportSpotCalculator.h"


SupportSpotCalculator::~SupportSpotCalculator()
{
	SAFE_DELETE( m_pRegulator );
}


SupportSpotCalculator::SupportSpotCalculator(int           numX,
											 int           numY,
											 SoccerTeam*   team):m_pBestSupportingSpot(NULL),
											 m_pTeam(team)
{
	const Region* PlayingField = team->Pitch()->PlayingArea();

	//calculate the positions of each sweet spot, create them and 
	//store them in m_Spots
	double HeightOfSSRegion = PlayingField->Height() * 0.8;
	double WidthOfSSRegion  = PlayingField->Width() * 0.9;
	double SliceX = WidthOfSSRegion / numX ;
	double SliceY = HeightOfSSRegion / numY;

	double left  = PlayingField->Left() + (PlayingField->Width()-WidthOfSSRegion)/2.0 + SliceX/2.0;
	double right = PlayingField->Right() - (PlayingField->Width()-WidthOfSSRegion)/2.0 - SliceX/2.0;
	double top   = PlayingField->Top() + (PlayingField->Height()-HeightOfSSRegion)/2.0 + SliceY/2.0;

	for (int x=0; x<(numX/2)-1; ++x)
	{
		for (int y=0; y<numY; ++y)
		{      
			if (m_pTeam->Color() == SoccerTeam::blue)
			{
				m_Spots.push_back(SupportSpot(Vector2D(left+x*SliceX, top+y*SliceY), 0.0));
			}

			else
			{
				m_Spots.push_back(SupportSpot(Vector2D(right-x*SliceX, top+y*SliceY), 0.0));
			}
		}
	}

	//create the regulator
	m_pRegulator = new Regulator(GetInstObj(CGameSetup).SupportSpotUpdateFreq);
}


//--------------------------- DetermineBestSupportingPosition -----------------
//
//  see header or book for description
//-----------------------------------------------------------------------------
Vector2D SupportSpotCalculator::DetermineBestSupportingPosition()
{
	//only update the spots every few frames                              
	if (!m_pRegulator->isReady() && m_pBestSupportingSpot)
	{
		return m_pBestSupportingSpot->m_vPos;
	}

	//reset the best supporting spot
	/// ������ѽ�Ӧ��
	m_pBestSupportingSpot = NULL;

	double BestScoreSoFar = 0.0;

	std::vector<SupportSpot>::iterator curSpot;

	for (curSpot = m_Spots.begin(); curSpot != m_Spots.end(); ++curSpot)
	{
		//first remove any previous score. (the score is set to one so that
		//the viewer can see the positions of all the spots if he has the 
		//aids turned on)
		/// ����ɾ����ǰ�ķ���
		curSpot->m_dScore = 1.0;

		//Test 1. is it possible to make a safe pass from the ball's position 
		//to this position?
		/// ���ȴ������λ���Ƿ�ȫ
		if(m_pTeam->isPassSafeFromAllOpponents(m_pTeam->ControllingPlayer()->Pos(),
			curSpot->m_vPos,
			NULL,
			GetInstObj(CGameSetup).MaxPassingForce))
		{
			curSpot->m_dScore += GetInstObj(CGameSetup).Spot_PassSafeScore;
		}

		/// ���������λ������
		//Test 2. Determine if a goal can be scored from this position.  
		if( m_pTeam->CanShoot(curSpot->m_vPos,            
			GetInstObj(CGameSetup).MaxShootingForce))
		{
			curSpot->m_dScore += GetInstObj(CGameSetup).Spot_CanScoreFromPositionScore;
		}   

		/// �����������Ա��Զ
		//Test 3. calculate how far this spot is away from the controlling
		//player. The further away, the higher the score. Any distances further
		//away than OptimalDistance pixels do not receive a score.
		if (m_pTeam->SupportingPlayer())
		{
			const double OptimalDistance = 200.0;

			double dist = Vec2DDistance(m_pTeam->ControllingPlayer()->Pos(),
				curSpot->m_vPos);

			double temp = fabs(OptimalDistance - dist);

			if (temp < OptimalDistance)
			{
				/// ��׼�����룬�����ӵ�������
				//normalize the distance and add it to the score
				curSpot->m_dScore += GetInstObj(CGameSetup).Spot_DistFromControllingPlayerScore *
					(OptimalDistance-temp)/OptimalDistance;  
			}
		}

		/// ��鵽Ŀǰλ��������Ƿ�����߷�
		//check to see if this spot has the highest score so far
		if (curSpot->m_dScore > BestScoreSoFar)
		{
			BestScoreSoFar = curSpot->m_dScore;

			m_pBestSupportingSpot = &(*curSpot);
		}    

	}

	return m_pBestSupportingSpot->m_vPos;
}





//------------------------------- GetBestSupportingSpot -----------------------
//-----------------------------------------------------------------------------
Vector2D SupportSpotCalculator::GetBestSupportingSpot()
{
	if (m_pBestSupportingSpot)
	{
		return m_pBestSupportingSpot->m_vPos;
	}

	else
	{ 
		return DetermineBestSupportingPosition();
	}
}

//----------------------------------- Render ----------------------------------
//-----------------------------------------------------------------------------
void SupportSpotCalculator::Render()const
{
	GetInstObj(CGDI).HollowBrush();
	GetInstObj(CGDI).GreyPen();

	for (unsigned int spt=0; spt<m_Spots.size(); ++spt)
	{
		GetInstObj(CGDI).Circle(m_Spots[spt].m_vPos, m_Spots[spt].m_dScore);
	}

	if (m_pBestSupportingSpot)
	{
		GetInstObj(CGDI).GreenPen();
		GetInstObj(CGDI).Circle(m_pBestSupportingSpot->m_vPos, m_pBestSupportingSpot->m_dScore);
	}
}