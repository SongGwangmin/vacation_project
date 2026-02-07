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

    // 팔방 이동에 따른 플레이어 회전
    ctx.updatePlayerRotation();


    // 상태 전환 로직
    if (!ctx.isMoving()) {
        ctx.changeState(std::make_unique<IdleState>());
    }
    else if (ctx.isJumping()) {
        ctx.changeState(std::make_unique<JumpingState>());
    }
    else if (ctx.ismouseleftdown()) {
        ctx.changeState(std::make_unique<ShootingState>());
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
    else if (ctx.ismouseleftdown()) {
        ctx.changeState(std::make_unique<ShootingState>());
	}
    else if (ctx.isJumping()) {
        ctx.changeState(std::make_unique<JumpingState>());
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

	std::cout << animationTime << std::endl;

	ctx.keydata->yVelocity = JUMP_VELOCITY; // 초기 점프 속도 설정
}

void JumpingState::update(Context& ctx) {
    std::cout << "점프 중..." << std::endl;
    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        //animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = (float)scene->mAnimations[animationIndex]->mDuration * 0.99f;
    }


    if (ctx.timeInTicks * 2 < (float)scene->mAnimations[animationIndex]->mDuration) {
        *(ctx.animtime) = ctx.timeInTicks * 2;
        
	}
    else {
        *(ctx.animtime) = (float)scene->mAnimations[animationIndex]->mDuration * 0.99f;
        if (ctx.ismouseleftdown()) {
            ctx.changeState(std::make_unique<ShootingState>());
        }
    }

    // 중력 적용
	ctx.keydata->yVelocity -= GRAVITY * ctx.deltaTime; // 중력 가속도 적용
	std::cout << "yVelocity: " << ctx.keydata->yVelocity << std::endl;
    // 상태 전환 로직
	
    if (ctx.keydata->yVelocity <= -JUMP_VELOCITY) {
        ctx.changeState(std::make_unique<FallingState>());
	}
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

	std::cout << animationTime << std::endl;
}

void FallingState::update(Context& ctx) {
    std::cout << "낙하 중..." << std::endl;

	bool istimerend = false; // 타이머 종료 여부 확인 변수

    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);

		istimerend = true; // 타이머 종료
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 중력 적용
    ctx.keydata->yVelocity -= 9.8f * ctx.deltaTime; // 중력 가속도 적용

    // 상태 전환 로직
    if (ctx.isground(istimerend)) {
		ctx.keydata->yVelocity = 0.0f; // 착지 시 속도 초기화
        ctx.changeState(std::make_unique<IdleState>());
    }
    else if (ctx.ismouseleftdown()) {
        ctx.changeState(std::make_unique<ShootingState>());
    }
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
    if (!ctx.ismouseleftdown()) {
        ctx.changeState(std::make_unique<IdleState>());
    }
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

	bool istimerend = false; // 타이머 종료 여부 확인 변수
    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
		istimerend = true; // 타이머 종료
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
    if (istimerend) {
        ctx.changeState(std::make_unique<SwiftState>());
    }
    else if (!ctx.ismouseleftdown()) {
        ctx.changeState(std::make_unique<IdleState>());
    }
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

    bool istimerend = false; // 타이머 종료 여부 확인 변수
    // animtime 먼저 갱신
    if (ctx.timeInTicks >= (float)scene->mAnimations[animationIndex]->mDuration) {
        animationTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        ctx.timeInTicks = fmod(ctx.timeInTicks, (float)scene->mAnimations[animationIndex]->mDuration);
		istimerend = true; // 타이머 종료
    }

    *(ctx.animtime) = ctx.timeInTicks;

    // 상태 전환 로직
    if (istimerend) {
        ctx.changeState(std::make_unique<DanglingState>());
	}
    else if (!ctx.ismouseleftdown()) {
        ctx.changeState(std::make_unique<IdleState>());
    }
}

void SwiftState::exit(Context& ctx) {
    std::cout << "스위프트 종료" << std::endl;
}