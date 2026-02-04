#pragma once

#include <iostream>
#include <memory>

#define Dangling 0
#define Falling 1
#define Idle 2
#define Jumping 3
#define Running 4
#define Shooting 5
#define Swift 6


class InputData {
public:
    int yVelocity;
    bool isMouseLeftDown;
    bool isLeftDown;
	bool isRightDown;
	bool isUpDown;
	bool isDownDown;
    bool isSpaceDown;

    InputData()
        :
		isMouseLeftDown(false), isLeftDown(false), isRightDown(false), isUpDown(false), isDownDown(false), isSpaceDown(false) {
		yVelocity = 0;
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

    void changeState(std::unique_ptr<State> newState) {
        if (currentState) {
            currentState->exit(*this);
        }
        currentState = std::move(newState);
        currentState->enter(*this);
    }

    void update(float* animtime, float& timeInTicks) {
        if (currentState) {
			this->animtime = animtime;
			this->timeInTicks = timeInTicks;
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