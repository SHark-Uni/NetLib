작성방법
[개요]
기본적인 틀을 잡아주는 코드 생성틀입니다. 
 - 내부의 컨텐츠나, 송신방법은 직접 컨텐츠에 따라 알맞게 수정하여야 합니다.
 - 작성된 txt파일은 exe파일과 같은 폴더에 'protocol_.txt'로 저장해주세요.
[타입 정의]
 - @typedef를 이용하면, Header의 최상단에 typedef 구문이 들어갑니다.
   ex)@typedef char MESSAGE_TYPE
 - @typedef를 통해서, 게임 프로토콜의 Message의 Type을 정해주세요.
	- default는 Short입니다.
 - @typedef를 통해서, SESSION_KEY를 정의하지 마세요. 
	- 네트워크 라이브러리에 SESSION_KEY는 이미 정의가 되어있습니다. 중복정의 에러가 발생합니다. 

[메시지 속성]
어노테이션느낌으로 작성합니다.
- 메시지는 2가지 타입이 있습니다. (@RES , @REQ)
   - @RES(서버->클라) : 클라의 수신부와 서버의 송신부를 만듬. 
   - @REQ(클라->서버) : 클라의 송신부와 서버의 수신부를 만듬.
   - 이 외에 타입은 허락하지 않습니다.
@RES인 경우, 추가옵션으로 3가지 통신방식에 따라, 생성되는 Send함수가 다릅니다. 
	-@Uni : Unicast입니다.
	-@Broad : BroadCast입니다.
	-@BroadExceptMe : 자신을 제외하고 BroadCast합니다.
	-속성은 3가지 중 하나만 정의합니다. 중복정의할 경우, 동작하지 않습니다.

[메시지 기본 형식]
아래는 기본적인 Format입니다. 

 ```
 @typedef short MESSAGE_TYPE
 @RES | @REQ
 if)@RES 
 @Uni | @Broad | @BroadExceptMe
 메시지 이름 : 메시지 번호
 {
	타입 변수이름,
	타입 변수이름,
	타입 변수이름,
	...
 }
 ```

 [OUTFILE]
 - 11개의 파일을 생성합니다.
 - MessageFormat.h : 작성한 프로토콜을 기반으로 `#define`을 한 메시지들을 생성해냅니다.
 - server/Client_SendMessageProc.h/cpp : 서버/클라 송신부 header와 cpp file입니다.(총 4개. 서버 header,cpp 클라 header, cpp)
 - server/Client_RecvMessageProc.h/cpp : 서버/클라 수신부 header와 cpp file입니다.(총 4개)
 - server/Client_OnRecvProc.cpp : 서버/클라 수신 시, switch - case되는 MessageProc() 입니다. (총 2개)

[주의점]
 - 메시지는 최대 600개까지 지원합니다.
 - @REQ는 방향성이 정해져있기 때문에, @Uni,@Broad등을 붙일 시 올바르게 작동하지 않습니다.
 - @typedef를 통해서, 게임 프로토콜의 Message의 Type을 정해주세요.
 - 작성된 txt파일은 exe파일과 같은 폴더에 'protocol_.txt'로 저장해주세요.
 - 같은 메시지에 대해서, 중복정의를 지원하지 않습니다. 
	- 중복정의를 할 경우, 클라이언트의 수신부 코드에 중복된 RES가 2개 나타나게 됩니다.
	- 각 메시지에 대해서, 다른 통신 속성을 주는 경우는 컨텐츠의 종류에 따라 예측이 불가능한 영역이기 때문에, 
	기본원칙은 하나의 통신 종류를 정하는 겁니다. (추후 변경 예정)
```
DO NOT THIS WAY
@RES
@Uni
MOVESTART : 1
{
	int id,
	short x,
	short y,
}

@RES
@Broad
MOVESTART : 1
{
	int id,
	short x,
	short y,
} 
```