 
//  File  :  /include/login.h
//
//  Login daemon's include macros
 
#define NEWS_FILE	"/adm/news/news"
#define MOTD_DIR	"/adm/motd/"
#define NPLAYER_INTRO	"/adm/news/nplayer_intro"
#define NPLAYER_NEWS	"/adm/news/nplayer_news"
 
//  The old unix-style login prompt
 
//#define LOGIN_MSG "\n" + version() + " (" + mud_name() + ")\n"
//#define LOGIN_PROMPT "\nlogin: "
//#define PASSWORD_PROMPT "password: "
 
//  Standard mud-style login prompt
 
#define LOGIN_MSG read_file("/adm/news/welcome") + "\n" + \
	capitalize(mud_name()) + "은(는) " + MUDLIB_VERSION + \
	" 머드 라이브러리, " + version() + "을(를) 실행 중입니다.\n\n"
#define LOGIN_PROMPT "어떤 이름으로 불리시겠습니까? "
#define PASSWORD_PROMPT "비밀번호를 입력해 주세요: "
 
 
//  If NO_REMOTE_LOGINS is defined, then the login system will compare the
//  incoming user's ip number to that of the local host's stored in the
//  mudlib's name server. If the first two number sets match (ie: x.x.*.*)
//  then the user will be allowed to login. This option is useful when first
//  setting up the mudlib to avoid unwanted external disturbances, or if you
//  want to limit your users to your institutional network.

//#define NO_REMOTE_LOGINS
 
 
//  If ADMIN_LOCK is defined, then the mud will be locked to all but
//  Admin, and the definition will be used as the explanation.
 
//#define ADMIN_LOCK "We've moved again, this time to: enterprise.shv.hb.se 5555."
 
 
//  If WIZ_LOCK is defined, then only those users with wizard status
//  are allowed to login.  If a definition is given, it will be used
//  as the reason for the lock out.

//#define WIZ_LOCK	"\tTesting a new lock process ... Try again shortly\n"
 
 
//  Define the number of users that may be logged on at one time. If any
//  more than that number try to log, they are locked out until there
//  there is a place for them online (Admin are not locked out).

#define MAX_USERS	50


//  define NO_NEW_USERS to disable new character creation.
//  Any value for this #define will be printed to the user's screen before
// hir connection is dumped.

#undef NO_NEW_USERS "\t새 기능을 시험 중입니다."
 
 
//  If USER_LIST is defined, a list of visible users will be given
//  during the login welcome screen.
 
#define USER_LIST
 
 
//  Define NO_SHUTDOWN_LOGIN if you wish to prevent non-admins from
//  logging in during a shutdown with under 5 minutes left. It will
//  inform them how long it will be before the mud is back up.
 
//define NO_SHUTDOWN_LOGIN
 
 
//  Define REGISTER_MSG if you wish all new users to register their
//  characters via email before they can play.  The defined message
//  will be displayed before the connection is closed.
 
#ifdef 0
#define REGISTER_MSG "현재 이 머드에서는 사전 등록된 사용자만 접속할 수 있습니다. 캐릭터를 받으려면\n" + \
	   ADMIN_EMAIL + "로 이메일을 보내 신청해 주세요.\n연락 기다리겠습니다.\n\n"
#endif
 
 
//  If you only want one Guest character to be online at one time,
//  then define ONE_GUEST, otherwise the number is unlimited.
 
#define ONE_GUEST
 
 
//  Define BANISHED_NAMES if you want the name-banning code held in
//  /adm/daemons/banish.c to function.
 
#define BANISHED_NAMES


//  Define BANISHED_SITES if you want the site-banning code held in
//  /adm/daemons/banish.c to function.  This does not affect the
//  functioning of the /adm/etc/access.allow file.

#define BANISHED_SITES
 
 
//  Define EMAIL_REGISTRATION if you want the mail-registry code held in
//  /adm/daemons/banish.c to function.

#define EMAIL_REGISTRATION
 
 
//  If NO_LOGIN_PAUSE is defined, then a pause will not be given during
//  the login process. Otherwise, a pause will be given after the
//  login news are displayed.
 
// #define NO_LOGIN_PAUSE
 
 
//  If SUPPRESS_ADMIN_LOGIN is defined, the list of "Current users" at
//  the login screen will not include admin members currently logged in.

// #define SUPPRESS_ADMIN_LOGIN
