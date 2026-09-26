// 파일   :  /adm/daemons/logind.c
// 작성자 :  Sulam@TMI  (12-13-91)
// 수정   :  Sulam@TMI  (3-29-92)  실명 입력 기능 추가
//     Sulam@TMI  (4-10-92)  게임 잠금 기능 추가
//     Buddha@TMI  (9-4-92)  뉴스 시스템 추가 및
//      로그인 절차 확장
//
// Buddha@TMI가 (12-92) 캐릭터 생성 모듈을 분리하고
// 로그인 데몬으로 동작하도록 전면 개편
//
// 수정   :  Watcher@TMI  (2-9-93)  뉴스 표시를 뉴스 데몬으로 이동하고
//      로그인 상태에서 멈춘 사용자를 확인하는 기능 추가
//     Watcher@TMI  (2-22-93)  환영 화면에 선택적으로
//      접속자 목록을 표시하는 기능 추가
//     Watcher@TMI  (2-23-93)  게스트 로그인 절차 간소화,
//      이름 차단 시스템 및 기타 기능 개선
//     Watcher@TMI  (3-9-93)  로그인 실패 알림 추가
//     Watcher@TMI  (4-7-93)  최대 접속자 수, WIZ_LOCK,
//      ADMIN_LOCK 및 서버 종료 중 접속 확인 추가
//     Watcher@TMI  (4/15/93)  NO_REMOTE_LOGIN 확인 옵션 추가
//     Watcher@TMI  (4/29/93)  휴면 상태 확인 기능 추가
//     Karathan  (7/13/93)  차단 관련 코드를 정리하고
//      차단 데몬으로 이동
//     Karathan  (8/12/93)  이메일 등록 명령 처리 기능 추가
//     Rust@TMI-2 (11/12/93)  WIZ_LOCK 버그 수정
//     Rust@TMI-2 (11/24/93) 새 캐릭터 생성 비활성화 옵션 추가
//     Mobydick@TMI-2 (4/8/94) 로그인 실패 기록 기능 추가
//			Inspiral@Tabor의 아이디어를 바탕으로 함
//     Inspiral@TMI-2 (04/18/94) log_file(BAD_LOGIN) 메시지 축약
//    Beek@TMI-2 (7/31/94) hushlogin 지원
//	Leto@Tmi-2 (6/24/95) SAVE_EXTENSION/__SAVE_EXTENSION__ 처리 수정

// 로그인 절차의 상당 부분은 머드의 종류와 분위기에 맞게
// 설정할 수 있습니다. 관련 설정은 대부분 /include/login.h에 있습니다.
 
#include <uid.h>
#include <priv.h>
#include <config.h>
#include <mailer.h>
#include <mudlib.h>
#include <login.h>
#include <daemons.h>
#include <login_macros.h>
#include <net/daemons.h>
#include <logs.h>
 
 
void logon(object ob);
string active_users();
protected void get_name(string str, object ob);
protected void get_password(string pass, object ob, int count);
protected int check_password(string pass, object ob);
protected void choice(string choice, object ob, string name);
protected void register_site();
protected void exec_old_copy(string s, object ob);
protected void login_new_copy(object ob);
protected void enter_world(object ob);
protected void check_email(object user);
 
 
void create()
{
    seteuid(ROOT_UID);
}
 
// 새 사용자가 로그인할 때 호출됩니다.
 
void logon(object ob)
{
    string local_host, pattern;
    int a1, a2, a3, a4;
	string s1,s2 ;
 
    if (base_name(previous_object()) != CONNECTION)
	return;
 
//  내부 네트워크에서의 접속만 허용하려면 /include/login.h에서
//  NO_REMOTE_LOGINS를 활성화합니다. 로그인한 사용자의 IP 주소 앞 두 부분이
//  머드 호스트의 IP 주소 앞 두 부분과 일치해야 접속할 수 있습니다.
#ifdef NO_REMOTE_LOGINS
    local_host = (string)DNS_MASTER->get_host_name(mud_name());
    if (!local_host || local_host == "" ||
	sscanf(local_host, "%d.%d.%d.%d", a1, a2, a3, a4) != 4)
      { write("\n  게임 서버 이름에서 네트워크 IP 주소를 확인할 수 없습니다.\n" +
          "  사용자 확인을 위해 접근 가능한 IP 주소가 필요합니다.\n" +
          "  연결을 종료합니다.\n\n");
	ob->remove_user();
	return; }
    else
	pattern = a1 + "." + a2 + ".%d.%d";
    if (!query_ip_number(ob) ||
	sscanf(query_ip_number(ob), pattern, a1, a2) != 2)
      { write("\n\t현재 " + capitalize(mud_name()) + "은(는) 내부 접속만 " +
          "허용하고 있습니다.\n\t문의는 다음 주소로 보내 주세요: " +
          ADMIN_EMAIL + "\n\n");
	ob->remove_user();
	return; }
#endif /* NO_REMOTE_LOGINS */
 
 
    write(LOGIN_MSG);
 
//  로그인 중 현재 접속자 목록을 표시하려면 /include/login.h에서
//  USER_LIST를 활성화합니다.
#ifdef USER_LIST
    write(wrap("현재 접속자: " + active_users()) + "\n");
#endif /* USER_LIST */
 
    write(LOGIN_PROMPT);
    input_to("get_name", 2, ob);
    return;
}
 
 
protected int valid_hangul_name(string name)
{
    int i, length, codepoint;

    length = sizeof(name);
    if (length < 1 || length > 6) return 0;
    for (i = 0; i < length; i++) {
        codepoint = name[i];
        if (codepoint < 0xAC00 || codepoint > 0xD7A3) return 0;
    }
    return 1;
}

protected void get_name(string str, object ob)
{
    string tmp, tmp1, tmp2, name_units;
    int bad_name, loop, i, sd_time, exists, korean_name;
 
// 이름을 입력했는지 확인합니다.
    if (!str || str=="") {
    write ("\n캐릭터 이름을 입력해야 합니다.\n\n") ;
	write (LOGIN_PROMPT) ;
	input_to("get_name",2,ob) ;
	return ;
    }
    // 여기서 종료하려는지 확인합니다.
	if (str == "q" || str == "quit")
	{
        write ("다음에 또 만나요!\n") ;
	    ob->remove_user() ;
	    return ;
	}
//  관리자만 머드에 접속하도록 제한하려면 /include/login.h에서
//  ADMIN_LOCK을 활성화하고 접속 제한 사유를 지정합니다.
#ifdef ADMIN_LOCK
    if (!adminp(str))
      { write("\n\n\n\t\t" + mud_name() +
          "은(는) 현재 점검을 위해 임시로 문을 닫았습니다.\n\n" +
          "\t\t점검 사유:\n" +
          "\t\t" + ADMIN_LOCK + "\n\n" +
          "\t\t이용에 불편을 드려 죄송합니다.\n" +
          "\t\t\t-운영진\n\n\n");
	ob->remove_user();
	return; }
#endif /* ADMIN_LOCK */
 
 
//  /include/login.h에 최대 접속자 수가 지정되어 있으면 제한을 초과했는지
//  확인합니다. 관리자가 아니라면 나중에 다시 접속하도록 안내합니다.
#ifdef MAX_USERS
    if (!adminp(str) && sizeof(users()) > MAX_USERS)
    { write("\n\n  죄송합니다. 현재 " + capitalize(mud_name()) +
	      "에 접속 인원이 가득 찼습니다. 잠시 후 다시 시도해 주세요.\n\n");
	ob->remove_user();
	return; }
#endif /* MAX_USERS */
 
 
//  /include/login.h에서 NO_SHUTDOWN_LOGIN을 정의하면 서버 종료가 진행 중일 때
//  관리자 외 사용자의 접속을 막고, 서버가 다시 열릴 예정 시각을 안내합니다.
#ifdef NO_SHUTDOWN_LOGIN
    sd_time = (int)SHUTDOWN_D->query_shutdown();
    if (sd_time)
	sd_time = (sd_time - time()) / 60;
    if (sd_time && sd_time < 5 && !adminp(str))
      { write("\n\n  죄송합니다. 현재 서버 종료 절차가 진행 중입니다.\n  " +
          "서버는 약 " + (sd_time + 3) + "분 후 다시 운영될 예정입니다.\n" +
          "  그때 다시 만나요!\n\n");
	ob->remove_user();
	return; }
#endif /* NO_SHUTDOWN_LOGIN */
 
    if (str == "")
      { ob->remove_user();
	return; }
 
    str = lower_case(str);
    exists = file_exists(user_data_file(ob, str) + __SAVE_EXTENSION__);
    if (!exists)
	exists = file_exists(PDATA_DIR + extract(str, 0, 0) + "/" + str +
		    __SAVE_EXTENSION__);
    korean_name = valid_hangul_name(str);
    name_units = "";
    for (i = 0; i < sizeof(str); i++)
        name_units += sprintf("%s%d", i ? "," : "", str[i]);
    log_file("account_create", sprintf("%s input name=%O bytes=%d units=%s hangul=%d exists=%d connection_file=%s\n",
        ctime(time()), str, strlen(str), name_units, korean_name, exists,
        user_data_file(ob, str) + __SAVE_EXTENSION__));
    if (exists && !korean_name) {
        if (strlen(str) > 11) {
            write("이름은 11자를 초과할 수 없습니다.\n");
            write(LOGIN_PROMPT);
            input_to("get_name", 2, ob);
            return;
        }
        for (i = 0; i < strlen(str); i++) {
            if (str[i] < 'a' || str[i] > 'z') {
                write("이름에는 영문자(a-z)만 사용할 수 있습니다.\n" +
		      "새 이름을 입력해 주세요: ");
                input_to("get_name", 2, ob);
                return;
            }
        }
    } else if (!korean_name && str != "guest") {
        write("새 캐릭터 이름은 한글 완성형 음절만 사용할 수 있습니다.\n");
        write(LOGIN_PROMPT);
        input_to("get_name", 2, ob);
        return;
    }
 
 
    //  사용자가 이미 존재하는지 확인하고, 새 이름이라면 유효성을 검사합니다.
    if (!exists)
      {
//  새 캐릭터 생성을 막으려면 NO_NEW_USERS를 활성화합니다.
#ifdef NO_NEW_USERS
    write("\n\n\n\t죄송합니다. 현재 "+capitalize(mud_name())+"에서는 새 캐릭터를 만들 수 없습니다.\n\n");
 if(NO_NEW_USERS)
write(NO_NEW_USERS);
 write("\n\n");
  ob->remove_user();
  return;
#endif /* NO_NEW_USERS */
 
//  마법사만 머드에 접속하도록 제한하려면 /include/login.h에서
//  WIZ_LOCK을 활성화합니다. WIZ_LOCK에 사유를 지정하면 접속 종료 전에 표시됩니다.
#ifdef WIZ_LOCK
    write("\n\n\n\t현재 "+capitalize(mud_name())+"은(는) 플레이어의 접속을 제한하고 있습니다.\n\n");
  if(WIZ_LOCK)
    write(WIZ_LOCK);
  write("\n\n");
  ob -> remove_user();
  return;
#endif /*WIZ_LOCK*/
 write("\n\"" + capitalize(str) + "\"은(는) 새 캐릭터 이름입니다.\n" +
	  "이 이름으로 생성하시겠습니까? (예/아니오): ");
	input_to("choice", 2, ob, str);
	return; }
 
    seteuid(str);
    export_uid(ob);
    seteuid(getuid());
    ob->SET_NAME(str);
    if (!ob->restore())
	  { write("로그인 정보를 복구하지 못했습니다.\n");
	ob->remove();
	return ; }
 
#ifdef WIZ_LOCK
    if (!ob->query("wizard"))
      { write("\n\n\n\t현재 " + capitalize(mud_name()) +
          "은(는) 플레이어의 접속을 제한하고 있습니다.\n\n");
	if (WIZ_LOCK)
	    write( WIZ_LOCK );
	write("\n\n");
	ob->remove_user();
	return; }
#endif /* WIZ_LOCK */
 
    // 게스트 캐릭터는 비밀번호를 묻지 않습니다.
    if (str == "guest")
      { get_password("", ob, 1);
	return; } 
    input_to("get_password", 3, ob, 1);
    write(PASSWORD_PROMPT);
    return;
}
 
 
protected void remove_copy(object ob)
{
    if (ob)
	ob->remove();
}
 
 
protected void get_password(string pass, object ob, int count)
{
    object body;
    int hibernate;
 
    write("\n");
    if (!check_password(pass, ob) && (string)ob->query("name") != "guest")
	  { write("비밀번호가 올바르지 않습니다.\n");
	if (count > 2)
      { write("\n비밀번호 입력 횟수를 초과했습니다.\n");
	    ob->set("passwd_fail", ({ query_ip_name(ob), time() }) );
#ifdef BAD_LOGIN
	    log_file(BAD_LOGIN,
ctime(time())[4..15] + " " + (string) ob -> query( "name" ) + " : " +
query_ip_name( ob ) + "\n" );
#endif
	    ob->remove_user();
	    return; }
    write("비밀번호를 다시 입력해 주세요: ");
	input_to("get_password", 3, ob, count + 1);
	return; }
 
    // 캐릭터가 휴면 상태인지 확인합니다.
    hibernate = ob->query("hibernate");
    if (hibernate && time() < hibernate)
	  { write("\n캐릭터가 " + ctime(hibernate) + "까지 휴면 상태입니다.\n" +
	       "해당 시각 이후 다시 로그인할 수 있습니다. 그때 뵙겠습니다!\n");
	ob->remove_user();
	return; }
    else if (hibernate)
	ob->set("hibernate", 0);
    body = find_player((string) ob->NAME);
 
    // 로그인 절차에서 멈춘 사용자가 있으면 이전 연결을 제거합니다.
    if (body && !environment(body))
	body->remove();
 
    if (body)
      { ob->SET_BODY_OB(body);
	if (interactive(body))
 
//  /include/login.h에서 ONE_GUEST를 정의하면 게스트는 한 명만 접속할 수 있습니다.
#ifdef ONE_GUEST
	  { if ((string)ob->query("name") == "guest")
          { write("이미 게스트 캐릭터가 접속 중입니다.\n" +
              "잠시 후 다시 시도하거나 새 캐릭터로 로그인해 주세요.\n\n");
		call_out("remove_copy",1, this_player());
		return; }
#else /* ONE_GUEST */
	  { if ((string)ob->query("name") == "guest")
	      { exec_old_copy("n", ob);
		return; }
#endif /* ONE_GUEST */
        write("\n같은 캐릭터로 접속한 다른 세션이 아직 활성화되어 있습니다.\n");
        write("기존 세션을 종료하시겠습니까? (예/아니오): ");
	    input_to("exec_old_copy", 2, ob);
	    return; }
 
//  /include/login.h에서 EXEC_COPY를 정의하면 연결 전환을 기록합니다.
#ifdef EXEC_COPY
	log_file(EXEC_COPY, capitalize((string)ob->NAME) +
		 " :\texec copy from " + query_ip_address(ob) + " [" +
		 extract(ctime(time()),4,15) + "]\n");
#endif /* EXEC_COPY */
 
	if (ob->connect())
	    body->restart_heart();
	else
      { write("재접속에 실패했습니다.\n");
	    ob->remove(); }
	return; }
    login_new_copy(ob);
}
 
 
protected
void login_new_copy(object ob)
{
    if (!ob->restore_body())
      { write("캐릭터 정보를 복구하지 못했습니다.\n" +
          "이 문제를 " + ADMIN_EMAIL + "로 알려 주시거나,\n" +
          "다른 이름으로 로그인해 운영진에게 알려 주세요.\n");
	ob->remove();
	return; }
    if (ob->connect())
	enter_world(ob);
    else
	  { write("로그인에 실패했습니다.\n");
	ob->remove(); }
}
 
 
protected
void exec_old_copy(string s, object user)
{
    object tmp, link;
    string old_ip;
 
    if (member_array(s, ({ "y", "Y", "Yes", "yes", "예" }) ) == -1)
      { if (!wizardp((object)user->BODY_OB) && 
	    (string)user->query("name") != "guest")
      { write("다음에 다시 접속해 주세요.\n");
	    user->remove();
	    return; }
 
//  /include/login.h에서 NEW_COPY를 정의하면 새 연결을 기록합니다.
#ifdef NEW_COPY
	log_file(NEW_COPY, capitalize((string)user->NAME) +
		 " :\tNew copy from " + query_ip_name(user) + " [" +
		 extract(ctime(time()),4,15) + "]\n");
#endif /* NEW_COPY */
 
	login_new_copy(user);
	return; }
    //  기존 연결 객체
    link = user->BODY_OB->query_link();
 
//  /include/login.h에서 FORCE_EXEC를 정의하면 강제 연결 전환을 기록합니다.
#ifdef FORCE_EXEC
    log_file(FORCE_EXEC, capitalize((string)user->NAME) + " :\tForce exec " +
	     "from " + query_ip_name(user) + " [ " +
	     extract(ctime(time()),4,15) + "]\n");
#endif /* FORCE_EXEC */
 
    tell_object(user->BODY_OB,
        "\n다른 위치(" + query_ip_name(user) + ")에서 접속하여\n" +
        "현재 캐릭터 세션이 종료되었습니다.\n");
    old_ip = user->BODY_OB->query("ip");

    tmp = new(CONNECTION);
    //  기존 객체에 새 연결을 넘깁니다.
    exec(tmp, user->BODY_OB);
    if (old_ip != query_ip_name(user) && old_ip != query_ip_number(user)) {
        user->BODY_OB->setup();
    }

    tmp->remove();
    if (user->connect())
    write("재접속했습니다.\n");
    else
    write("재접속에 실패했습니다.\n");
    link->remove();
    return;
}
 
 
protected void enter_world(object user)
{
    mixed *bad_pass;
 
    seteuid(geteuid(user));
    export_uid(user->BODY_OB);
    seteuid(getuid());
    bad_pass = (mixed *)user->query("passwd_fail");
    if (bad_pass)
      { tell_object(this_player(), "\n알림: " + bad_pass[0] + "에서 " +
            ctime(bad_pass[1]) + "에 로그인 실패가 있었습니다.\n\n");
	user->set("passwd_fail", 0); }
    check_email(user);
    user->BODY_OB->setup();
}
 
 
protected int check_password(string pass, object ob)
{
    string password;
 
    password = ob->PASS;
    if (password == crypt(pass, password))
	return 1;
    return 0;
}
 
 
protected void choice(string choice, object user, string name)
{
    int i;
    string pass;
 
    log_file("account_create", sprintf("%s confirmation=%s name=%s\n",
        ctime(time()), choice, name));
    write("\n");
    choice = lower_case(choice);
    if (member_array(choice, ({ "n", "no", "아니오" }) ) >= 0)
	  { write("알겠습니다. 이름을 다시 입력해 주세요: ");
	input_to("get_name", 2, user);
	return; }
    else if (member_array(choice, ({ "y", "yes", "예" }) ) == -1)
	  { write("잘못된 입력입니다. 예 또는 아니오를 입력해 주세요: ");
	input_to("choice", user, name);
	return; }
 
//  /include/login.h에서 EMAIL_REGISTRATION을 정의하면 차단 데몬에서
//  사전 등록된 이름과 비밀번호를 확인합니다.
#ifdef EMAIL_REGISTRATION
    pass = BANISH_D->check_mailreg_name(name);
#else
    pass = "";
#endif
 
//  /include/login.h에서 REGISTER_MSG를 정의하면 이 단계에서 신규 로그인을
//  허용하지 않습니다. REGISTER_MSG에는 미등록 사용자의 접속을 막는 사유를
//  설명하는 메시지를 지정합니다.
#ifdef REGISTER_MSG
    if (!pass || !stringp(pass) || pass == "")
      { write(REGISTER_MSG);
	call_out("remove_copy", 1, user);
	return; }
#endif /* REGISTER_MSG */
 
//  /include/login.h에서 BANISHED_SITES를 정의하면 차단 데몬을 통해
//  접속 위치가 차단되었는지 확인합니다.
#ifdef BANISHED_SITES
    //  접속 위치 차단 코드는 Dainia@DreamShadow가 작성했습니다.
    //  Karathan이 7/13/93에 수정했습니다.
    if (!pass || !stringp(pass) || pass == "")
      { i = BANISH_D->check_banned_site(query_ip_number());
	if (i < 0)
      { write("\n  사용자 컴퓨터의 네트워크 IP 주소를 확인할 수 없습니다.\n" +
          "  사용자 확인을 위해 접근 가능한 IP 주소가 필요합니다.\n" +
          "  연결을 종료합니다.\n\n");
 	   user->remove_user();
 	   return; }
	if (i)
      { write("\n\n\t" + mud_name() + " 운영진이 현재 접속 위치의 등록을 요구하고 있습니다.\n" +
          "\t캐릭터를 신청하려면 다음 주소로 이메일을 보내 주세요.\n" +
          "\t\t" + ADMIN_EMAIL + "\n\n\t\t\t-운영진\n");
	    user->remove_user();
	    return; } }
#endif /* BANISHED_SITES */
 
 
//  /include/login.h에서 BANISHED_NAMES를 정의하면 차단 데몬을 통해
//  캐릭터 이름이 차단되었는지 확인합니다.
#ifdef BANISHED_NAMES
    if (BANISH_D->check_banned_name(name))
	  { write("해당 캐릭터 이름은 사용할 수 없습니다.\n\n" +
	      LOGIN_PROMPT);
	input_to("get_name", 2, user);
	return; }
#endif /* BANISHED_NAMES */
 
    seteuid(name);
    export_uid(user);
    seteuid(getuid());
    user->SET_NAME(name);
    log_file("account_create", sprintf("%s approved name=%s; calling newuserd\n",
        ctime(time()), name));
    NEWUSER_D->create_new_user(user, pass);
    return;
}
 
 
//  사용자에게 새 이메일이 도착했는지 확인합니다.
void check_email(object user)
{
    mapping mail_stat;
    int toread;
 
  if( file_exists( user_mbox_file( user->NAME ) + __SAVE_EXTENSION__ ) ) {
    mail_stat = (mapping) MAILBOX_D->mail_status(user->NAME);
    toread = mail_stat["unread"];
    if (toread)
    printf("\n새 편지가 %d통 도착했습니다.\n\n", toread);
  }
}
 
 
//  접속자 표시 기능이 선택된 경우, 로그인 중인 사용자에게
//  현재 접속 중인 공개 사용자 목록을 보여 줍니다.
string active_users()
{
    mixed *who;
    string output;
    int loop;
 
    who = users() ;
    who = filter_array(who, "filter_invis", this_object());
    if (!who || !sizeof(who))
	return "None.";
    who = map_array(who, "switch_name", this_object());
    who = uniq_array(who) ;
    if (sizeof(who) == 1)
	output = who[0] + ".";
    else
	output = implode(who[0..sizeof(who)-2], ", ") + ", and " +
		 who[sizeof(who)-1] + ".";
    return output;
}
 
 
protected int filter_invis(object who)
{
    if (!who || !environment(who))
    // 로그인 중인 사용자는 목록에 표시하지 않습니다.
	return 0;
    if ((int)who->query("npc"))
    // 몬스터는 목록에 표시하지 않습니다.
	return 0;
#ifdef SUPPRESS_ADMIN_LOGIN
    if (adminp(geteuid(who)))
        return 0;
#endif
    return visible(who);
}
 
 
//  사용자 객체를 이름 문자열로 변환합니다.
protected string switch_name(object who)
{
    return capitalize((string)who->query("name")) ;
}

