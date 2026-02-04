#include "statemachine.h"
#include "model.h"
#include <gl/freeglut.h>

// ========================================================
// RunningState 구현
// ========================================================
void RunningState::enter(Context& ctx) {
    std::cout << "달리기 시작" << std::endl;
    animationIndex = ANIM_Running;
    animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void RunningState::update(Context& ctx) {
    std::cout << "달리는 중..." << std::endl;

    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
    if (!ctx.isMoving()) {
        ctx.changeState(std::make_unique<IdleState>());
    }
}

void RunningState::exit(Context& ctx) {
    std::cout << "달리기 멈춤" << std::endl;
}


// ========================================================
// IdleState 구현
// ========================================================
void IdleState::enter(Context& ctx) {
    animationIndex = ANIM_Idle;
    animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    std::cout << "대기 시작" << std::endl;
}

void IdleState::update(Context& ctx) {
    std::cout << "대기 중..." << std::endl;

    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
    if (ctx.isMoving()) {
        ctx.changeState(std::make_unique<RunningState>());
    }
}

void IdleState::exit(Context& ctx) {
    std::cout << "대기 종료" << std::endl;
}


// ========================================================
// JumpingState 구현
// ========================================================
void JumpingState::enter(Context& ctx) {
    std::cout << "점프 시작" << std::endl;
    animationIndex = ANIM_Jumping;
    animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void JumpingState::update(Context& ctx) {
    std::cout << "점프 중..." << std::endl;
    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
}

void JumpingState::exit(Context& ctx) {
    std::cout << "점프 종료" << std::endl;
}


// ========================================================
// FallingState 구현
// ========================================================
void FallingState::enter(Context& ctx) {
    std::cout << "낙하 시작" << std::endl;
    animationIndex = ANIM_Falling;
    animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void FallingState::update(Context& ctx) {
    std::cout << "낙하 중..." << std::endl;

    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
}

void FallingState::exit(Context& ctx) {
    std::cout << "낙하 종료" << std::endl;
}


// ========================================================
// DanglingState 구현
// ========================================================
void DanglingState::enter(Context& ctx) {
    std::cout << "매달리기 시작" << std::endl;
    animationIndex = ANIM_Dangling;
    animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void DanglingState::update(Context& ctx) {
    std::cout << "매달리는 중..." << std::endl;

    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
}

void DanglingState::exit(Context& ctx) {
    std::cout << "매달리기 종료" << std::endl;
}


// ========================================================
// ShootingState 구현
// ========================================================
void ShootingState::enter(Context& ctx) {
    std::cout << "사격 시작" << std::endl;
    animationIndex = ANIM_Shooting;
    animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void ShootingState::update(Context& ctx) {
    std::cout << "사격 중..." << std::endl;

    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
}

void ShootingState::exit(Context& ctx) {
    std::cout << "사격 종료" << std::endl;
}


// ========================================================
// SwiftState 구현
// ========================================================
void SwiftState::enter(Context& ctx) {
    std::cout << "스위프트 시작" << std::endl;
    animationIndex = ANIM_Swift;
    animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void SwiftState::update(Context& ctx) {
    std::cout << "스위프트 중..." << std::endl;

    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
}

void SwiftState::exit(Context& ctx) {
    std::cout << "스위프트 종료" << std::endl;
}