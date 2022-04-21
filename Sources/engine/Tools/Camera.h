#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>

class Camera
{	
		enum Movement
		{
			DOLLY		= 0x01,
			TRUCK		= 0x02,
			PEDESTAL	= 0x04,

			PAN			= 0x08,
			TILT		= 0x10,
			ROLL		= 0x20
		};
		
		short mDollyCounter;
		short mTruckCounter;
		short mPedestalCounter;

		short mPanCounter;
		short mTiltCounter;
		short mRollCounter;

		const short DOLLY_DURATION = 40;
		const float MAX_DOLLY_OFFSET = 0.05f;
		
		const short TRUCK_DURATION = 30;
		//const float MAX_TRUCK_OFFSET = 0.03f;

		const short PEDESTAL_DURATION = 30;
		//const float MAX_PEDESTAL_OFFSET = 0.03f;

		const short PAN_DURATION = 30;
		//const float MAX_PAN_OFFSET = 0.03f;

		const short TILT_DURATION = 30;
		//const float MAX_TILT_OFFSET = 0.03f;

		const short ROLL_DURATION = 30;
		//const float MAX_ROLL_OFFSET = 0.03f;

		unsigned char mCurrentMove;

		bool mIsDrag = false;
		bool mIsPitch = false;

		glm::vec2 mNDCBeginMove;
		glm::vec2 mNDCCurrentMove;
		glm::vec2 mNDCElapsed;

		float mDollyFactor;
		
		glm::vec3 mPos;
		glm::vec3 mLook;
		glm::vec3 mUp;
		glm::vec3 mRight;

		float mFov;
		float mNear;
		float mFar;
		
		float mWidth;
		float mHeight;
		
		glm::mat4 mViewMat;
		glm::mat4 mProjectMat;
		
	public:
		Camera();
		~Camera();
	
		void Init(const glm::vec2& winSize, const glm::vec3& pos, const glm::vec3& look, const glm::vec3& up);
		void SetPosition(const glm::vec3& pos);
		void SetOrientationByLookAndUp(const glm::vec3& look, const glm::vec3& up);
		void SetOrientationByLookAndRight(const glm::vec3& look, const glm::vec3& right);

		const glm::vec3& GetPosition() const { return mPos; }
		const glm::vec3& GetLook() const { return mLook; }
		const glm::vec3& GetUp() const { return mUp; }
		
		const glm::mat4& GetViewMat() const { return mViewMat; }
		const glm::mat4& GetProjectMat() const { return mProjectMat; }
		
		void Update();
		
		void BeginDrag(float x, float y);
		void Drag(float x, float y);
		void EndDrag();
		void BeginPitch(float x, float y);
		void Pitch(float x, float y);
		void EndPitch();
		void Wheel(float val);
		
		void Resize(const glm::vec2& winSize);
		
};

#endif