#include "Camera.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

Camera::Camera(){}
Camera::~Camera(){}

void Camera::Init(const glm::vec2& winSize, const glm::vec3& pos, const glm::vec3& look, const glm::vec3& up)
{	
	mCurrentMove = 0;
	mFov = 60.f;
	mNear = 0.01f;
	mFar = 10.f;
	
	mProjectMat = glm::mat4(1.f);
	mViewMat = glm::mat4(1.f);
	mPos = pos;
	
	Resize(winSize);
	SetOrientationByLookAndUp(look, up);
}
void Camera::SetPosition(const glm::vec3& pos)
{
	mPos = pos;
	
	mViewMat[3][0] = -glm::dot(mRight, mPos);
	mViewMat[3][1] = -glm::dot(mUp, mPos);
	mViewMat[3][2] = glm::dot(mLook, mPos);
}
void Camera::SetOrientationByLookAndUp(const glm::vec3& look, const glm::vec3& up)
{
	mLook = glm::normalize(look);
	mRight = glm::normalize(glm::cross(mLook, up));
	mUp = glm::cross(mRight, mLook);
	
	mViewMat[0][0] = mRight.x; mViewMat[0][1] = mUp.x; mViewMat[0][2] = -mLook.x;
	mViewMat[1][0] = mRight.y; mViewMat[1][1] = mUp.y; mViewMat[1][2] = -mLook.y;
	mViewMat[2][0] = mRight.z; mViewMat[2][1] = mUp.z; mViewMat[2][2] = -mLook.z;
	
	SetPosition(mPos);
}

void Camera::SetOrientationByLookAndRight(const glm::vec3& look, const glm::vec3& right)
{
	mLook = glm::normalize(look);
	mUp = glm::normalize(glm::cross(mRight, mLook));
	mRight = glm::cross(mLook, mUp);

	mViewMat[0][0] = mRight.x; mViewMat[0][1] = mUp.x; mViewMat[0][2] = -mLook.x;
	mViewMat[1][0] = mRight.y; mViewMat[1][1] = mUp.y; mViewMat[1][2] = -mLook.y;
	mViewMat[2][0] = mRight.z; mViewMat[2][1] = mUp.z; mViewMat[2][2] = -mLook.z;

	SetPosition(mPos);
}

void Camera::Resize(const glm::vec2& winSize)
{
	mWidth = winSize.x;
	mHeight = winSize.y;
	
	float aspect = (float)mWidth / (float)mHeight;

	mProjectMat[0][0] = 1.f / (aspect * tanf(mFov * (float)M_PI / 180.f / 2.f));
	mProjectMat[1][1] = 1.f / tanf(mFov * (float)M_PI / 180.f / 2.f);
	mProjectMat[2][2] = -(mFar + mNear) / (mFar - mNear);
	mProjectMat[2][3] = -1.f;
	mProjectMat[3][2] = -(2.f * mFar * mNear) / (mFar - mNear);
}

void Camera::BeginDrag(float x, float y)
{
	mIsDrag = true;
	
	mNDCBeginMove.x = 2.f * ((float)x / mWidth) - 1.f;
	mNDCBeginMove.y = 1.f - 2.f * ((float)y / mHeight);
	mNDCCurrentMove = mNDCBeginMove;
}
void Camera::Drag(float x, float y)
{
	if(mIsDrag)
	{
		mNDCBeginMove = mNDCCurrentMove;
		mNDCCurrentMove.x = 2.f * ((float)x / mWidth) - 1.f;
		mNDCCurrentMove.y = 1.f - 2.f * ((float)y / mHeight);

		mNDCElapsed = mNDCCurrentMove - mNDCBeginMove;

		if (mNDCElapsed.x != 0.f)
		{
			mCurrentMove |= PAN;
			mPanCounter = (int)PAN_DURATION;
		}
		if (mNDCElapsed.y != 0.f)
		{
			mCurrentMove |= TILT;
			mTiltCounter = (int)TILT_DURATION;
		}
	}
}
void Camera::EndDrag()
{
	mIsDrag = false;
}
void Camera::BeginPitch(float x, float y)
{
	mIsPitch = true;
	
	mNDCBeginMove.x = 2.f * ((float)x / mWidth) - 1.f;
	mNDCBeginMove.y = 1.f - 2.f * ((float)y / mHeight);
	mNDCCurrentMove = mNDCBeginMove;
}
void Camera::Pitch(float x, float y)
{
	if(mIsPitch)
	{
		mNDCBeginMove = mNDCCurrentMove;
		mNDCCurrentMove.x = 2.f * ((float)x / mWidth) - 1.f;
		mNDCCurrentMove.y = 1.f - 2.f * ((float)y / mHeight);
	}
}
void Camera::EndPitch()
{
	mIsPitch = false;
}

void Camera::Wheel(float val)
{
	if (val > 0.f)
		mDollyFactor = 1.f;
    else if (val < 0.f)
		mDollyFactor = -1.f;

	mCurrentMove |= DOLLY;

	mDollyCounter = (int)DOLLY_DURATION;
}

static glm::vec3 rotateVectorAroundAxis(const glm::vec3& vec, float angle, const glm::vec3& axis)
{
	float c = cos(glm::radians(angle));
	float s = sin(glm::radians(angle));
	return vec * c + (glm::dot(vec, axis) * axis) * (1.f - c) + glm::cross(axis, vec) * s;
}

void Camera::Update()
{
	if (mCurrentMove & DOLLY)
	{
		glm::vec3 new_pos;
		float delta = mDollyFactor * (MAX_DOLLY_OFFSET / (DOLLY_DURATION * DOLLY_DURATION)) * mDollyCounter * mDollyCounter;

		new_pos = mPos + delta * mLook;

		SetPosition(new_pos);
	}

	if (mCurrentMove & PAN)
	{
		float delta = (40.f * mNDCElapsed.x / (PAN_DURATION * PAN_DURATION)) * mPanCounter * mPanCounter;
		mLook = rotateVectorAroundAxis(mLook, delta, glm::vec3(0.f, 1.f, 0.f));
		mUp = rotateVectorAroundAxis(mUp, delta, glm::vec3(0.f, 1.f, 0.f));
		SetOrientationByLookAndUp(mLook, mUp);
	}

	if (mCurrentMove & TILT)
	{
		float delta = -(40.f * mNDCElapsed.y / (TILT_DURATION * TILT_DURATION)) * mTiltCounter * mTiltCounter;
		mLook = rotateVectorAroundAxis(mLook, delta, mRight);
		SetOrientationByLookAndRight(mLook, mRight);
	}

	
	if (mDollyCounter > 0)
		mDollyCounter--;
	else
		mCurrentMove &= ~DOLLY;

	if (mPanCounter > 0)
		mPanCounter--;
	else
		mCurrentMove &= ~PAN;
		
	if (mTiltCounter > 0)
		mTiltCounter--;
	else
		mCurrentMove &= ~TILT;
		
}