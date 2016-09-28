#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef _WIN32
    #define     VER1                2
    #define     VER2                9
    #define     VER3                2
    #define     VER4                29
#else
    #define     VER1                2
    #define     VER2                9
    #define     VER3                2
#define     VER4                73
#endif

#define LOG_LEVEL_DEBUG
#if 0
#define    VERSION_JSJQ
//请不要更改此行之前的代� �,更改江苏军区版本号请修改cmessagebox.cpp中的edition//; 
#define LOG_LEVEL_DEBUG //LOG_LEVEL_ERROR   //define log level

//#define VERSION_DCN
//#define VERSION_DHC
//#define VERSION_OEM
//#define VERSION_FRONWARE
//#define VERSION_VSAP  //启明星辰 need FAP -->VSAP
#ifdef VERSION_OEM
#ifdef _WIN32
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_oem2.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_venus.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_DCN.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_ inspur.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_ sugon.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_dhc.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/vClient_configure_title_OEM.png"
#else
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_venus.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_DCN.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_OEM.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_sugon.png"
//#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_DHC.png"

#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_OEM.png"
#endif
#endif

#ifdef VERSION_DHC // 东华
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_dhc.png"
#define CONFIG_TITLE_IMG ":/image/resource/image/vclient_login_background_dhc.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_DHC.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_DHC.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/DHC-vclient.png"
#endif

#ifdef VERSION_DCN //神马
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_DCN.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_DCN.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_DCN.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_DCN.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_DCN_VCLASS //神马vclass
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_DCN_vclass.png"
#else
#define VERSION_VCLASS
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_vclass.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_vclass.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_DCN_vclass.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_SUGON  //曙光
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_SUGON.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_SUGON.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_SUGON.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_SUGON_2000  //曙光
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_SUGON.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_SUGON_2000.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_SUGON_2000.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_SUGON_2000.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_ONLY //昂利
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_ONLY.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_ONLY.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_ONLY.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_VSAP  //启明星辰
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_VENUS.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_VENUS.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_VENUS.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_INSPUR //浪潮
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_INSPUR.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_INSPUR.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_INSPUR.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_DCITS//神州信息
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_DCITS.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_DCITS.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_DCITS.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_NOLOGO //
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_NOLOGO.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_NOLOGO.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_NOLOGO.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_FRONWARE //方物
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_FRONWARE.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_FRONWARE.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_FRONWARE.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_FRONWARE.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_FRONWARE_VCLASS//方物vclass
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_FRONWARE_vclass.png"
#else
#define VERSION_VCLASS
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_vclass.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_vclass.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_FRONWARE_vclass.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_HXXY//华夏星云
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_HXXY.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_HXXY.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_HXXY.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_HXXY.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_LENOVO_VCLASS //联想
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_LENOVO.png"
#else
#define VERSION_VCLASS
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_LENOVO_vclass.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_LENOVO_vclass.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_LENOVO_vclass.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif
#ifdef VERSION_LENOVO //联想桌面虚拟化
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_LENOVO.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_LENOVO.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_LENOVO.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_LENOVO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_XRBR //鑫荣博锐
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_XRBR.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_XRBR.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_XRBR.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_NOLOGO.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#endif

#ifdef VERSION_JSJQ //江苏军区
#ifdef _WIN32
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/vclient_login_background_JSJQ.png"
#else
#define LOGIN_DLG_BACKGROUNT_IMG ":image/resource/image/fronview_login_background_JSJQ.png"
#define CONFIG_TITLE_IMG    ":/image/resource/image/Fronview_configure_title_JSJQ.png"
#define IMAGE_FIRSTLOADING_BACKGROUND ":/image/resource/image/background_JSJQ.png"
#endif
#define ABOUT_IMAGE     ":image/resource/image/about_vCl_logo.png"
#define PRODUCT_NAME_IMG_PATH ":image/resource/image/jsjq_title.png"
#define COMPANY_LOGO  ":image/resource/image/logo_jsjq.png"
#endif

#define WINDOWS_ICON    ":image/resource/image/about_vCl_logo.png"
#define VERTICAL_LINE_PIC   ":image/resource/image/vertical_line.png"
#define CORNER_TOP_LEFT_IMG ":/image/resource/image/corner_left_top.png"

#ifdef VERSION_VCLASS
#define IMAGE_LOADING_MOVIE ":image/resource/image/firstloading_blue.gif"
#define STYLE_SHEET_TEXT "font: 12pt \"微软雅黑\";"
#else
//#define IMAGE_LOADING_MOVIE ":image/resource/image/firstloading_green.gif"
//#define STYLE_SHEET_TEXT "font: 12pt \"微软雅黑\"; color:#8bc600;"
#ifdef VERSION_JSJQ
#define IMAGE_LOADING_MOVIE ":image/resource/image/firstloading_blue.gif"
#define STYLE_SHEET_TEXT "font: 12pt \"微软雅黑\";"
#else
#define IMAGE_LOADING_MOVIE ":image/resource/image/firstloading_green.gif"
#define STYLE_SHEET_TEXT "font: 12pt \"微软雅黑\";"
#endif
#endif
#endif

#define STYLE_SHEET_RADIOBTN "QRadioButton::indicator::unchecked{"\
"image: url(\":/image/resource/image/radio_unchecked.png\")}"\
"QRadioButton::indicator::checked{"\
"image: url(\":/image/resource/image/radio_checked.png\")}"

#define STYLE_SHEET_COMBO_BOX "QComboBox{"\
"border: 1px groove #999999;"\
"border-radius: 2px;"\
"padding-top: 0px;"\
"padding-left: 3px;"\
"}"\
"QComboBox:on {"\
"padding-top: 0px;"\
"padding-left: 3px;"\
"}"

#define STYLE_SHEET_COMBO_BOX_OTHER "QComboBox{"\
"border: 1px groove #999999;"\
"border-radius: 2px;"\
"padding-top: 0px;"\
"padding-left: 3px;"\
"}"\
"QComboBox:on {"\
"padding-top: 0px;"\
"padding-left: 3px;"\
"}"\
"QComboBox::drop-down {"\
"width: 23px"\
"}"

#define STYLE_SHEET_PUSHBTN "QPushButton{"\
    "border:1px groove #999999;"\
    "width: 70;"\
    "height: 25;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFFFFF, stop: 1 #e9e9e9)"\
    "}"\
    "QPushButton:default{"\
    "border:1px groove #999999;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFFFFF, stop: 1 #e9e9e9)"\
    "}"\
    "QPushButton:hover{"\
    "border:1px groove #999999;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: white;"\
    "}"\
    "QPushButton:pressed{"\
    "border:1px groove #999999;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #d9d9d9, stop: 1 #e9e9e9)"\
    "}"

#endif //__CONFIG_H__
