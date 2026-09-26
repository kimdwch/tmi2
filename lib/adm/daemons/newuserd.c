// 파일 : /adm/daemons/newuserd.c
//
// 캐릭터 생성 모듈입니다.
// 사용자가 아직 사용되지 않은 이름으로 캐릭터를 만들면 logind.c에서
// 이 데몬을 호출하고, 사용자의 선택에 따라 새 캐릭터를 설정합니다.
// Buddha가 1992년 9월경 작업을 시작했습니다.
//
// Mobydick이 10-11-92에 종족, 기술, 능력치를 추가했습니다.
// Buddha가 1992년 12월과 1993년 1월에 기능을 확장했습니다.
//
// Psyche@TMI-2 (93/01/27)가 성별과 종족 선택 시
// 첫 글자 입력을 허용하도록 수정했습니다.
//
// Watcher@TMI가 (2/23/93) 절차를 간소화하고 선택 항목을 추가했습니다.
//
// Watcher@TMI가 (4/15/93) AUTO_WIZHOOD 옵션을 추가했습니다.
//
// Megadeath@TMI-2가 언어 기능을 추가했습니다.
//
// Karathan이 (8/12/93) 이메일 등록 기능을 추가했습니다.
//
// 94-11-09 : Leto가 언어 정보를 string * 대신 매핑으로 수정했습니다.
// 95-04-30 : Blue가 RACES를 <user2.h>의 #define으로 변경했습니다.
//            성별 목록도 추후 같은 방식으로 변경할 예정입니다.
// 96-01-14 : Dm이 GENDERS를 <user2.h>의 #define으로 변경하고,
//            이전 RACES와 동일하게 동작하도록 코드를 수정했습니다.

 
#include <logs.h>
#include <uid.h>
#include <priv.h>
#include <mudlib.h>
#include <config.h>
#include <login.h>
#include <login_macros.h>
#include <language.h>
#include <daemons.h>
#include <user2.h>
// 종족 목록이 newuserd에 정의된 이유는 알 수 없지만, 일단 그대로 둡니다.

protected void set_skills(object player);
protected void set_stats(object player);


void create()
{
    seteuid(ROOT_UID);
}


void create_new_user(object user, string pass)
{
    object body;

    if (geteuid(previous_object()) != ROOT_UID) return;
	log_file("account_create", sprintf("%s create_new_user name=%s connection_file=%s\n",
		ctime(time()), user->NAME,
		user_data_file(user) + __SAVE_EXTENSION__));
    user->SET_BODY(USER_OB); // let's assume a standard login
    body = new(USER_OB);

    body->set("name", (string)user->NAME, MASTER_ONLY);
    body->set ("cap_name", capitalize((string)user->NAME), MASTER_ONLY) ;
    body->set("snoopable", 1, MASTER_ONLY);

    user->SET_BODY_OB(body);
	log_file("account_create", sprintf("%s body-created name=%s body_file=%s\n",
		ctime(time()), user->NAME,
		user_data_file(body) + __SAVE_EXTENSION__));

    cat(NPLAYER_INTRO);

//  /include/login.h에서 EMAIL_REGISTRATION을 정의한 경우,
//  이 이름이 미리 비밀번호가 지정된 등록 이름인지 확인합니다.
#ifdef EMAIL_REGISTRATION
    if (pass && stringp(pass) && pass != "")
	  { write("등록 승인 시 받은 비밀번호를 입력해 주세요.\n");
	input_to("get_pass", 3, pass, user, 0);
	return; }
#endif  /* EMAIL_REGISTRATION */

	write("캐릭터에 사용할 비밀번호를 입력해 주세요. 비밀번호는 1자 이상 256자 이하여야 합니다.\n"+
   "컴퓨터 계정에서 사용하는 비밀번호나 사전에 있는 단어는 사용하지 마세요.\n") ;
    input_to("new_pass", 3, user, 0);
}


protected void new_pass(string pass, object user, int count)
{
		if (!pass || strlen(pass) < 1 || strlen(pass) > 256)
			{ write("\n비밀번호는 1자 이상 256자 이하여야 합니다.\n");
	if (count > 2)
	  { write("\n입력 횟수를 초과했습니다.\n");
	    user->remove_user();
	    return; }
	write("비밀번호를 다시 입력해 주세요: ");
	input_to("new_pass", 3, user, count + 1);
	return; }
	write("\n비밀번호를 다시 입력해 확인해 주세요: ");
    input_to("new_pass2", 3, pass, user, count);
}


protected void new_pass2(string pass2, string pass, object user, int count)
{
    if (pass == pass2)
      { user->SET_PASS(crypt(pass2, 0));
	printf("\n\n성별을 선택해 주세요:\n   %-=60s\n", implode(GENDERS, ", "));
	input_to("new_gender", 2, user, (object)user->BODY_OB, 0);
	return; }
	write("\n비밀번호가 일치하지 않습니다.\n");
    if (count > 2)
	  { write("\n입력 횟수를 초과했습니다.\n");
	user->remove_user();
	return; }
	write("캐릭터 비밀번호를 입력해 주세요: ");
    input_to("new_pass", 3, user, count + 1);
}


protected void get_pass(string pass, string prev, object user, int count)
{
    if (crypt(pass, prev) != prev)
	{ write("비밀번호가 올바르지 않습니다.\n");
	if (count > 2)
	  { write("\n입력 횟수를 초과했습니다.\n");
	    user->remove_user();
	    return; }
	write("비밀번호를 다시 입력해 주세요: ");
	input_to("get_pass", 3, prev, user, count + 1);
	return; }
    user->SET_PASS(prev);
//    write("\n\n성별을 선택해 주세요." + "\n성별을 입력해 주세요: ");
	printf("\n성별을 선택해 주세요:\n   %-=60s\n", implode(GENDERS, ", "));
    input_to("new_gender", 2, user, (object)user->BODY_OB, 0);
}


protected void new_gender(string g, object user, object body, int count) {
    string *rs;
    if (!g || sizeof(rs = regexp(GENDERS, "^"+g)) != 1) {
//    if (!g || member_array(g, ({"male", "female", "neuter", "hermaphrodite",
//    				"m", "f", "n", "h" })) == -1)
//      { write("\n성별은 male, female, neuter, hermaphrodite 중에서 선택해 주세요.\n");
	  printf("\n선택 가능한 성별:\n   %-=60s\n", implode(GENDERS, ", "));
	if (count > 2)
	  { write("\n입력 횟수를 초과했습니다.\n");
	    user->remove_user();
	    return; }
	write("성별을 입력해 주세요: ");
	input_to("new_gender", user, body, count + 1);
	return; }

// Psyche@TMI-2 (93/27/01): 한 글자 입력을 해당하는 전체 단어로 변환합니다.
// 이런 날짜 표기 방식은 처음 봅니다. 다시 사용하지 말아 주세요.
// 참고로 Blue가 남긴 날짜는 950430입니다.

//    switch(g)
//      { case "m": g = "male";
//		  break;
//	case "f": g = "female";
//		  break;
//	case "n": g = "neuter";
//		  break;
//	case "h": g = "hermaphrodite";
//		  break; }
    body->set("gender", g, READ_ONLY);
	printf("\n선택 가능한 종족:\n   %-=60s\n", implode(RACES, ", "));
//  write ("\n종족은 human, elf, dwarf, gnome, orc 중에서 선택할 수 있습니다.\n") ;
	write ("종족을 입력해 주세요: ") ;
    input_to("new_race", user, body, 0);
    return ;
}


protected void new_race(string r, object user, object body, int count)
{
    string *rs;
    if (!r || sizeof(rs = regexp(RACES, "^"+r)) != 1) {
//  if (!r || member_array(r, ({"elf", "dwarf", "gnome", "human", "orc",
// 				"e", "d", "g", "h", "o" })) == -1)
//    { write("\n종족은 human, elf, dwarf, gnome, orc 중에서 선택해야 합니다.\n");
	  printf("\n선택 가능한 종족:\n   %-=60s\n", implode(RACES, ", "));
	if (count > 2)
	  { write("\n입력 횟수를 초과했습니다.\n");
	    user->remove_user();
	    return; }
	write ("종족을 입력해 주세요: ") ;
	input_to("new_race", user, body, count + 1);
	return; }
//  switch(r)
//    { case "h": r = "human";
//		//break;
//	case "e": r = "elf";
//		//break;
//	case "d": r = "dwarf";
//		//break;
//	case "g": r = "gnome";
//		//break;
//	case "o": r = "orc";
//		//break; }
    body->set("race",r);
	write("\n이메일 주소를 입력해 주세요 (user@host): ");
    input_to("new_email", user, body, 0);
}


protected void new_email(string e, object user, object body, int count)
{
    string id, host;

    if (sscanf(e, "%s@%s", id, host) != 2 || id == "" || host == "")
	  { write("이메일 주소는 user@host 형식으로 입력해야 합니다.\n");
	if (count > 2)
	  { write("\n입력 횟수를 초과했습니다.\n");
	    user->remove_user();
	    return; }
	write("이메일 주소를 다시 입력해 주세요: ");
	input_to("new_email", user, body, count + 1);
	return; }
	if(e=="user@host") write("참 독창적이네요... 알겠습니다.\n");
    user->SET_EMAIL(e);
	write("실명을 입력해 주세요: ");
    input_to("get_real_name", user, body);
}


protected void get_real_name(string rn, object user, object body)
{
	string connection_file, character_file;

    if (!rn || rn == "")  rn = "???";
    user->SET_RNAME(rn);
 
	// 필요한 정보를 모두 받았으니 사용자를 게임에 접속시킵니다.
	// 능력치 굴림 등은 여기서 처리하지 않습니다. -- buddha
 
    set_stats(body);
    set_skills(body);
 
    seteuid(geteuid(user));
    export_uid(body);
    seteuid(getuid());
    user->connect();
 
    cat(NPLAYER_NEWS);

//  /include/logs.h에서 NEW_USER를 정의하면 캐릭터 생성 시각을 기록합니다.
#ifdef NEW_USER
    log_file(NEW_USER, capitalize((string)user->NAME) + " was created on " +
	     extract(ctime(time()), 4, 15) + " from " + query_ip_name() + ".\n");
#endif /* NEW_USER */
 
//  /include/config.h에서 AUTO_WIZHOOD를 정의하면 새 사용자에게 자동으로
//  마법사 권한을 부여하고 해당 설정에 지정된 PATH를 적용합니다.
//  TMI처럼 마법사 권한을 자유롭게 부여하는 곳에서 유용합니다.
#ifdef AUTO_WIZHOOD
    user->set("wizard", 1);
    body->set("PATH", AUTO_WIZHOOD);
	write("\t[마법사 권한이 자동으로 부여되었습니다]\n");
#endif /* AUTO_WIZHOOD */
 
    body->setup();
	connection_file = user_data_file(user) + __SAVE_EXTENSION__;
	character_file = user_data_file(body) + __SAVE_EXTENSION__;
	user->save_data_conn();
	log_file("account_create", sprintf("%s connection-save name=%s file=%s exists=%d\n",
		ctime(time()), user->NAME, connection_file, file_exists(connection_file)));
    body->save_data();
	log_file("account_create", sprintf("%s character-save name=%s file=%s exists=%d\n",
		ctime(time()), user->NAME, character_file, file_exists(character_file)));

//  /include/login.h에서 EMAIL_REGISTRATION을 정의한 경우,
//  등록된 이름을 등록 파일에서 제거합니다.
#ifdef EMAIL_REGISTRATION
    (void)BANISH_D->remove_mailreg_name(user->NAME);
#endif /* EMAIL_REGISTRATION */
}


int clean_up()
{
    destruct(this_object()); 
    return 1;
}


protected void set_stats(object player) {

	int strength, intelligence, dexterity, constitution ;
	int hp, sp, total ;
	// 이전 방식에서는 언어 목록을 문자열 배열로 저장했습니다.
	mapping languages; // Leto
	mapping stat ;
	// 이전 방식에서는 빈 문자열 배열로 초기화했습니다.
	languages = ([]); // Leto
	stat = allocate_mapping(4);

	total = 0 ;
	while (total<50 || total > 70) {
		strength = 9+random(7)+random(7) ;
		intelligence = 9+random(7)+random(7) ;
		dexterity = 9+random(7)+random(7) ;
		constitution = 9+random(7)+random(7) ;
		total = strength + intelligence + dexterity + constitution ;
	}
	switch (player->query("race")) {
		case "human" : {
			languages = ([ "human" : 100 ]) ;
			break ;
		}
		case "elf" : {
			intelligence = intelligence + 3 ;
			constitution = constitution - 2 ;
			dexterity = dexterity + 1 ;
			strength = strength - 2 ;
			languages = ([ "elvish" : 100 ]) ;
			break ;
		}
		case "dwarf" : {
			intelligence = intelligence - 1 ;
			dexterity = dexterity - 2 ;
			constitution = constitution + 2 ;
			strength = strength + 1 ;
			languages = ([ "dwarvish" : 100 ]) ;
			break ;
		}
		case "gnome" : {
			strength = strength - 3 ;
			dexterity = dexterity + 3 ;
			constitution = constitution - 1 ;
			intelligence = intelligence + 1 ;
			languages = ([ "gnomish" : 100 ]) ;
			break ;
		}
		case "orc" : {
			strength = strength + 3 ;
			constitution = constitution + 1 ;
			dexterity = dexterity - 1 ;
			intelligence = intelligence - 3 ;
			languages = ([ "orcish" : 100 ]) ;
		}
	}
	stat["strength"] = strength ;
	stat["intelligence"] = intelligence ;
	stat["dexterity"] = dexterity ;
	stat["constitution"] = constitution ;
	player->set("stat", stat, LOCKED) ;

#ifdef LANGUAGES
	languages += ([ "common" : 100 ]) ;
	player->set("languages", languages);
#endif
	hp = 25 + constitution + random(21) ;
	sp = 45 + intelligence + random(21) ;
	player->set("hit_points", hp) ;
	player->set("max_hp", hp, MASTER_ONLY);
	player->set("spell_points", sp) ;
	player->set("max_sp", sp, LOCKED);
	return ;
}

protected void set_skills(object player) {
	player->wipe_skills() ;
 player->set_skill("Thrusting weapons",0,"strength") ;
 player->set_skill("Cutting weapons",0,"strength") ;
 player->set_skill("Blunt weapons",0,"strength") ;
 player->set_skill("Parrying defense",0,"dexterity") ;
 player->set_skill("Shield defense",0,"dexterity") ;
 player->set_skill("Combat spells",0,"intelligence") ;
 player->set_skill("Healing spells",0,"intelligence") ;
 player->set_skill("Divinations spells",0,"intelligence") ;
	player->set_skill("Wilderness",0,"dexterity") ;
 player->set_skill("First aid",0,"dexterity") ;
 player->set_skill("Theft",0,"dexterity") ;
 player->set_skill("Stealth",0,"dexterity") ;
	return ;
}
