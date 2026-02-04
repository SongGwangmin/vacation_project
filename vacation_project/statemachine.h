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
    GLuint yVelocity;
    bool isMouseLeftDown;
    bool isLeftDown;
	bool isRightDown;
	bool isUpDown;
	bool isDownDown;
    bool isSpaceDown;

    InputData()
        : yVelocity(0),
		isMouseLeftDown(false), isLeftDown(false), isRightDown(false), isUpDown(false), isDownDown(false), isSpaceDown(false) {
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
};

// 구체적인 상태 구현
class RunningState : public State {
public:
    void enter(Context& ctx) override { std::cout << "달리기 시작" << std::endl; }
    void update(Context& ctx) override {
        std::cout << "달리는 중..." << std::endl; 
    }
    void exit(Context& ctx) override { std::cout << "달리기 멈춤" << std::endl; }
};

class IdleState : public State {
public:
    void enter(Context& ctx) override { 
        animationIndex = Idle;
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        std::cout << "대기 시작" << std::endl; 
    }
    void update(Context& ctx) override {
        std::cout << "대기 중..." << std::endl;
        // 예시: 조건 만족 시 상태 변경
        // ctx.changeState(std::make_unique<RunningState>());

		// animtime 먼저 갱신
        if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
			animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
			ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);

        }
        
        *(ctx.animtime) = ctx.timeInTicks;
        

		// 상태 전환 로직

    }
    void exit(Context& ctx) override { std::cout << "대기 종료" << std::endl; }
};

class JumpingState : public State {
    public:
    void enter(Context& ctx) override { std::cout << "점프 시작" << std::endl; }
    void update(Context& ctx) override {
        std::cout << "점프 중..." << std::endl; 
    }
    void exit(Context& ctx) override { std::cout << "점프 종료" << std::endl; }
};

class FallingState : public State {
    public:
    void enter(Context& ctx) override { std::cout << "낙하 시작" << std::endl; }
    void update(Context& ctx) override {
        std::cout << "낙하 중..." << std::endl; 
    }
    void exit(Context& ctx) override { std::cout << "낙하 종료" << std::endl; }
};

class DanglingState : public State {
    public:
    void enter(Context& ctx) override { std::cout << "매달리기 시작" << std::endl; }
    void update(Context& ctx) override {
        std::cout << "매달리는 중..." << std::endl; 
    }
    void exit(Context& ctx) override { std::cout << "매달리기 종료" << std::endl; }
};

class ShootingState : public State {
    public:
    void enter(Context& ctx) override { std::cout << "사격 시작" << std::endl; }
    void update(Context& ctx) override {
        std::cout << "사격 중..." << std::endl; 
    }
    void exit(Context& ctx) override { std::cout << "사격 종료" << std::endl; }
};

class SwiftState : public State {
    public:
    void enter(Context& ctx) override { std::cout << "스위프트 시작" << std::endl; }
    void update(Context& ctx) override {
        std::cout << "스위프트 중..." << std::endl; 
    }
    void exit(Context& ctx) override { std::cout << "스위프트 종료" << std::endl; }
};