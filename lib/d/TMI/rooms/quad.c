// config.h only included here for the defines of START and VOID
// it doesn't need to be included in most rooms.
#include <config.h>
#include <mudlib.h>

inherit ROOM;

void create()
{
  ::create();
  seteuid(getuid());
  set( "light", 1 );
  set( "short", "유명한 쿼드" );
  // NB: "@EndText" should not have spaces after it.
  // "EndText" should be on a line of its own (no indentation, tabs or spaces)
  set( "long", @EndText
이곳은 새롭게 단장한 유명한 TMI-2 쿼드입니다.
여러 게시판 방으로 이어지는 임시 구역입니다.
남쪽에는 Intermud-3 같은 네트워크 관련 주제를 다루는 네트워크 방이,
북쪽에는 드라이버 문제를 논의하는 MudOS 방이 있습니다.
동쪽에는 TMI-2 머드 라이브러리의 버그를 신고하는 방이 있습니다.
아래로 내려가면 Fooland입니다.

뚜렷한 출구: 서쪽, 아래, 공허, 동쪽, 북쪽, 남쪽.
EndText
  );
  set( "exit_suppress", 1 );
  set( "exits", ([
   "down" : "/d/Fooland/hall",
 "east" : "/d/TMI/rooms/bugroom",
 "north" : "/d/TMI/rooms/driverroom",
  "south" : "/d/TMI/rooms/networkroom",
  "west" : "/d/TMI/rooms/archiveroom",
    "void"  : VOID,
  ]) );

call_other("/d/TMI/boards/generalboard","dsfjkdgfjksf");
reset();
}
