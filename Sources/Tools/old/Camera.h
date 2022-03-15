#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>


const float MOUSE_SENSITIVITY = 1.f;

class Camera
{
private:
    bool mIsDragging;
    bool mIsPinching;

    glm::vec3 mPos;
    glm::vec3 mUp;
    glm::vec3 mLook;

    glm::quat mOrientation;

    
    float mPitch;
    float mYaw;
    float mRoll;

    glm::mat4 mMatRot;
    glm::mat4 mMatTran;

    glm::mat4 mView;

    glm::quat mQBallFirst;
    glm::quat mQBallLast;

    glm::vec3 mBallCenter;

    glm::vec3 mBallFirst;
    glm::vec3 mBallSecond;

    glm::vec2 mNDCFirst;
    glm::vec2 mNDCSecond;

    float mBallRadius;

public:
    Camera() : mBallRadius(0.5f), mIsDragging(false), mIsPinching(false)
    {
        SetState(glm::vec3(0.f, 0.f, -2.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
        //mBallCenter = mPos + mLook * mBallRadius;
        mBallCenter = glm::vec3();
    }
    ~Camera() {}

    void SetState(const glm::vec3& pos, const glm::vec3& look, const glm::vec3& up)
    {

        mPos = pos;
        mLook = normalize(look);
        glm::vec3 right = normalize(cross(mLook, up));
        mUp = cross(right, mLook);

        

        //rotation
        mMatRot = glm::mat4(1.f);

        mMatRot[0][0] = right.x;  mMatRot[0][1] = mUp.x;  mMatRot[0][2] = -mLook.x; 
        mMatRot[1][0] = right.y;  mMatRot[1][1] = mUp.y;  mMatRot[1][2] = -mLook.y; 
        mMatRot[2][0] = right.z;  mMatRot[2][1] = mUp.z;  mMatRot[2][2] = -mLook.z; 

        //mOrientation = glm::toQuat(mMatRot);
        mQBallFirst = mQBallLast = glm::toQuat(mMatRot);
        
        mPitch = glm::degrees(glm::pitch(mOrientation));
        mYaw = glm::degrees(glm::yaw(mOrientation));
        mRoll = glm::degrees(glm::roll(mOrientation));


        //translation
        mMatTran = glm::mat4(1.f);

        mMatTran[3][0] = -mPos.x;   mMatTran[3][1] = -mPos.y;   mMatTran[3][2] = -mPos.z;

        //update view mat
        mView = mMatRot * mMatTran;
    }


    const glm::vec3& GetPosition() { return mPos; }


    void BeginDrag(const glm::vec2 &ndc)
    {
        mIsDragging = true;
        mNDCFirst = ndc;
        PointOnSphere(&mBallFirst, ndc);
    }

    void Drag(const glm::vec2 &ndc)
    {
        if (mIsDragging)
        {

            PointOnSphere(&mBallSecond, ndc);
            mBallFirst = glm::normalize(mBallFirst);
            mBallSecond = glm::normalize(mBallSecond);
            glm::quat q_drag = glm::rotation(mBallFirst, mBallSecond);

            mQBallLast = q_drag * mQBallFirst;

            mMatRot = glm::toMat4(mQBallLast);

            glm::vec4 pos = glm::vec4(mPos, 1.f);
            //pos.x = pos.x - mBallCenter.x;
            //pos.y = pos.y - mBallCenter.y;
            //pos.z = pos.z - mBallCenter.z;
            pos = mMatRot * pos;
            //pos.x = pos.x + mBallCenter.x;
            //pos.y = pos.y + mBallCenter.y;
            //pos.z = pos.z + mBallCenter.z;

            mPos = glm::vec3(pos.x, pos.y, pos.z);

            mBallCenter = mPos + mLook * mBallRadius;

            mLook.x = -mMatRot[0][2];
            mLook.y = -mMatRot[1][2];
            mLook.z = -mMatRot[2][2];

            mUp.x = mMatRot[0][1];
            mUp.y = mMatRot[1][1];
            mUp.y = mMatRot[2][1];

            

            mMatTran = glm::mat4(1.f);

            mMatTran[3][0] = -mPos.x;
            mMatTran[3][1] = -mPos.y;
            mMatTran[3][2] = -mPos.z;

            mView = mMatRot * mMatTran;

            //UpdateRotation();
            //UpdateTranslation();
        }
    }

    void EndDrag()
    {
        mIsDragging = false;
        mBallFirst = mBallSecond = {};
        mQBallFirst = mQBallLast;
    }

    void BeginPinch(const glm::vec2& ndc)
    {
        mIsPinching = true;
        mNDCFirst = ndc;
    }

    void Pinch(const glm::vec2& ndc)
    {
        if (mIsPinching)
        {
            glm::vec3 right = glm::cross(mLook, mUp);

            mNDCSecond = ndc;

            mPos -= right * (mNDCSecond.x - mNDCFirst.x) * 2.f;
            mPos -= mUp * (mNDCSecond.y - mNDCFirst.y) * 2.f;

            mNDCFirst = mNDCSecond;

            UpdateTranslation();
        }
    }

    void EndPinch()
    {
        mIsPinching = false;
    }

    void Wheel(float delta)
    {
        mPos += (delta / 4.f) * mLook;
        //mBallCenter = mPos + mLook * mBallRadius;
        UpdateTranslation();
    }

    
    const glm::mat4& GetView() { return mView; }


private:

    void UpdateRotation()
    {
        mMatRot = glm::mat4(1.f);
        mMatRot = glm::toMat4(mOrientation);

        mLook.x = -mMatRot[0][2];
        mLook.y = -mMatRot[1][2];
        mLook.z = -mMatRot[2][2];

        mUp.x = mMatRot[0][1];
        mUp.y = mMatRot[1][1];
        mUp.y = mMatRot[2][1];

        mView = mMatRot * mMatTran;
    }

    void UpdateTranslation()
    {
        mMatTran = glm::mat4(1.f);

        mMatTran[3][0] = -mPos.x;
        mMatTran[3][1] = -mPos.y;
        mMatTran[3][2] = -mPos.z;

        //mBallCenter = mPos + mLook * mBallRadius;

        mView = mMatRot * mMatTran;
    }


    void PointOnSphere(glm::vec3 *pointOnSphere, const glm::vec2 &ndc)
    {
        glm::vec2 screen(ndc);
        if (glm::dot(screen, screen) >= 1.f)
            screen = glm::normalize(screen);
        pointOnSphere->x = screen.x;
        pointOnSphere->y = screen.y;
        float sub_sqrt = 1.f - screen.x * screen.x - screen.y * screen.y;
        if (sub_sqrt >= 0.f)
            pointOnSphere->z = -sqrtf(1.f - screen.x * screen.x - screen.y * screen.y);
        else
            pointOnSphere->z = 0.f;
    }
};

#endif