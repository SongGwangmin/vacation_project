# OpenGL 라이브러리 설치 및 설정 가이드

## 현재 상황
프로젝트에서 다음 라이브러리를 사용하고 있지만 링크되지 않았습니다:
- GLEW (OpenGL Extension Wrangler)
- FreeGLUT (OpenGL Utility Toolkit)
- GLM (OpenGL Mathematics)

## 해결 방법

### 방법 1: vcpkg 사용 (가장 권장)

1. **vcpkg 설치** (아직 설치하지 않은 경우)
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

2. **필요한 라이브러리 설치**
   ```powershell
   cd C:\vcpkg
   .\vcpkg install glew:x64-windows
   .\vcpkg install freeglut:x64-windows
   .\vcpkg install glm:x64-windows
   ```

3. **Visual Studio 재시작** 후 프로젝트 빌드

### 방법 2: 수동 설치

#### GLEW 설치
1. http://glew.sourceforge.net/ 에서 다운로드
2. 압축 해제 후:
   - `include/GL/` 폴더를 `C:\OpenGL\include\GL\` 에 복사
   - `lib/Release/x64/glew32.lib` 를 `C:\OpenGL\lib\` 에 복사
   - `bin/Release/x64/glew32.dll` 를 프로젝트 실행 파일과 같은 폴더에 복사

#### FreeGLUT 설치
1. https://www.transmissionzero.co.uk/software/freeglut-devel/ 에서 다운로드
2. 압축 해제 후:
   - `include/GL/` 폴더를 `C:\OpenGL\include\GL\` 에 복사
   - `lib/x64/freeglut.lib` 를 `C:\OpenGL\lib\` 에 복사
   - `bin/x64/freeglut.dll` 를 프로젝트 실행 파일과 같은 폴더에 복사

#### GLM 설치
1. https://github.com/g-truc/glm/releases 에서 다운로드
2. 압축 해제 후:
   - `glm/` 폴더를 `C:\OpenGL\include\glm\` 에 복사
   (GLM은 헤더 온리 라이브러리이므로 .lib 파일이 필요 없습니다)

### 방법 3: 프로젝트 내부에 라이브러리 포함

프로젝트 폴더 구조:
```
vacation_project/
├── vacation_project/
│   ├── include/
│   │   └── GL/
│   │       ├── glew.h
│   │       ├── freeglut.h
│   │       └── glm/ (전체 폴더)
│   ├── lib/
│   │   ├── glew32.lib
│   │   └── freeglut.lib
│   └── main.cpp
```

이 경우 수정된 vcxproj 파일이 자동으로 `$(ProjectDir)include` 와 `$(ProjectDir)lib` 경로를 참조합니다.

## 프로젝트 설정 확인

수정된 `vacation_project.vcxproj` 파일에는 다음이 추가되었습니다:

1. **Include 디렉터리**:
   - `$(ProjectDir)`
   - `$(ProjectDir)include`
   - `C:\OpenGL\include`

2. **Library 디렉터리**:
   - `$(ProjectDir)lib`
   - `C:\OpenGL\lib`

3. **링커 추가 종속성**:
   - `glew32.lib`
   - `freeglut.lib`
   - `opengl32.lib`

## 코드 수정 사항

헤더 파일 경로를 다음과 같이 사용:
```cpp
#include <GL/glew.h>        // 대문자 GL
#include <GL/freeglut.h>
#include <glm/glm.hpp>       // 소문자 glm
```

또는

```cpp
#include <gl/glew.h>        // 소문자 gl
#include <gl/freeglut.h>
#include <gl/glm/glm.hpp>
```

## 실행 시 DLL 파일

프로그램 실행 시 다음 DLL 파일들이 필요합니다:
- `glew32.dll`
- `freeglut.dll`

이 파일들을 다음 중 한 곳에 위치시켜야 합니다:
1. 실행 파일(.exe)과 같은 폴더
2. `C:\Windows\System32` (권장하지 않음)
3. 시스템 PATH에 포함된 폴더

## 빌드 테스트

Visual Studio에서:
1. 솔루션 다시 로드 (필요한 경우)
2. 빌드 → 솔루션 다시 빌드
3. 오류 확인

## 문제 해결

### 링크 오류가 계속 발생하는 경우
1. Visual Studio를 재시작
2. 프로젝트 속성 → C/C++ → 일반 → 추가 포함 디렉터리 확인
3. 프로젝트 속성 → 링커 → 일반 → 추가 라이브러리 디렉터리 확인
4. 프로젝트 속성 → 링커 → 입력 → 추가 종속성 확인

### DLL 관련 오류
실행 파일이 있는 폴더에 DLL 파일을 복사:
```powershell
Copy-Item "C:\OpenGL\bin\glew32.dll" ".\x64\Debug\"
Copy-Item "C:\OpenGL\bin\freeglut.dll" ".\x64\Debug\"
```

## 백업
원본 프로젝트 파일은 `vacation_project.vcxproj.backup`으로 백업되었습니다.
