#pragma once

#include <iostream>
#include <memory>

#define ANIM_Dangling 0
#define ANIM_Falling 1
#define ANIM_Idle 2
#define ANIM_Jumping 3
#define ANIM_Running 4
#define ANIM_Shooting 5
#define ANIM_Swift 6

#define GRAVITY 9.8f
#define JUMP_VELOCITY 10.0f

class InputData {
public:
    float yVelocity;
    bool isMouseLeftDown;
    bool isLeftDown;
	bool isRightDown;
	bool isUpDown;
	bool isDownDown;
    bool isSpaceDown;

    InputData()
        :
		isMouseLeftDown(false), isLeftDown(false), isRightDown(false), isUpDown(false), isDownDown(false), isSpaceDown(false) {
		yVelocity = 0.0f;
	}
};

// 전방 선언
class Context;

// 상태 인터페이스
class State {
public:
    virtual ~State() = default;
    virtual void enter(Context& ctx) = 0;
    virtual void update(Context& ctx) = 0;
    virtual void exit(Context& ctx) = 0;
};

// Context (상태 머신 본체)
class Context {
    std::unique_ptr<State> currentState;
    

public:
    InputData* keydata;
    float* animtime;
    float timeInTicks;
	float deltaTime;

    void changeState(std::unique_ptr<State> newState) {
        if (currentState) {
            currentState->exit(*this);
        }
        currentState = std::move(newState);
        currentState->enter(*this);
    }

    void update(float* animtime, float& timeInTicks, float& deltaTime) {
        if (currentState) {
			this->animtime = animtime;
			this->timeInTicks = timeInTicks;
			this->deltaTime = deltaTime;
            currentState->update(*this);
        }
    }

    void init(std::unique_ptr<State> startState) {
        changeState(std::move(startState));
    }

    void setInputData(InputData* data) {
        keydata = data;
	}

    int isMoving() {
		int left = keydata->isLeftDown * -1;
		int right = keydata->isRightDown;
		int forward = keydata->isUpDown * -1;
		int backward = keydata->isDownDown;

		int horizontal = left + right;
		int vertical = forward + backward;
		return horizontal * horizontal + vertical * vertical;
	}

    bool ismouseleftdown() {
		return keydata->isMouseLeftDown;
	}

    int isJumping() {
        return keydata->isSpaceDown;
	}

    bool isground(bool istimerend){
        // 간단히 yVelocity가 음수이면 땅에 닿았다고 가정
        return (keydata->yVelocity <= 0.0f) && istimerend;
	}
};

// 구체적인 상태 구현

// 전방 선언

class RunningState : public State {
public:
    void enter(Context& ctx) override;
    void update(Context& ctx) override;
    void exit(Context& ctx) override;
};

class IdleState : public State {
public:
    void enter(Context& ctx) override;
    void update(Context& ctx) override;
    void exit(Context& ctx) override;
};

class JumpingState : public State {
public:
    void enter(Context& ctx) override;
    void update(Context& ctx) override;
    void exit(Context& ctx) override;
};

class FallingState : public State {
public:
    void enter(Context& ctx) override;
    void update(Context& ctx) override;
    void exit(Context& ctx) override;
};

class DanglingState : public State {
public:
    void enter(Context& ctx) override;
    void update(Context& ctx) override;
    void exit(Context& ctx) override;
};

class ShootingState : public State {
public:
    void enter(Context& ctx) override;
    void update(Context& ctx) override;
    void exit(Context& ctx) override;
};

class SwiftState : public State {
public:
    void enter(Context& ctx) override;
    void update(Context& ctx) override;
    void exit(Context& ctx) override;
};