#ifndef _CAMERA_FRUSTUM_H_
#define _CAMERA_FRUSTUM_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraFrustum
{
protected:
    float mNear;
    float mFar;

public:
    CameraFrustum() {}
    virtual ~CameraFrustum() {}

    virtual glm::mat4 GetProjectionMat() = 0;

    void SetNear(float near) { mNear = near; }
    float GetNear() { return mNear; }
    void SetFar(float far) { mFar = far; }
    float GetFar() { return mFar; }

    void SetRange(float near, float far) { mNear = near, mFar = far; }
    void SetRange(glm::vec2 near_far) { mNear = near_far.x, mFar = near_far.y; }
};

class Orthographic : public CameraFrustum
{
private:
    float mLeft;
    float mRight;
    float mBottom;
    float mTop;

public:
    Orthographic() : mLeft(-1), mRight(1), mBottom(-1), mTop(1)
    {
    }
    ~Orthographic() {}

    void SetLeft(float left) { mLeft = left; }
    float GetLeft() { return mLeft; }
    void SetRight(float right) { mRight = right; }
    float GetRight() { return mRight; }
    void SetBottom(float bottom) { mBottom = bottom; }
    float GetBottom() { return mBottom; }
    void SetTop(float top) { mTop = top; }
    float GetTop() { return mTop; }

    void SetScope(float left, float right, float bottom, float top)
    {
        mLeft = left, mRight = right, mBottom = bottom, mTop = top;
    }
    void SetScope(glm::vec4 scope)
    {
        mLeft = scope.x, mRight = scope.y, mBottom = scope.z, mTop = scope.w;
    }

    virtual glm::mat4 GetProjectionMat()
    {
        return glm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
    }
};

class Perspective : public CameraFrustum
{
private:
    float mFieldOfView;
    float mAspectRatio;

public:
    Perspective() {}
    ~Perspective() {}

    void SetFieldOfView(float fov) { mFieldOfView = fov; }
    float GetFieldOfView() { return mFieldOfView; }

    void ComputeAspectRatio(float width, float height)
    {
        if (height > 0)
            mAspectRatio = width / height;
    }

    virtual glm::mat4 GetProjectionMat()
    {
        return glm::perspective(glm::radians(mFieldOfView), mAspectRatio, mNear, mFar);
    }
};
#endif