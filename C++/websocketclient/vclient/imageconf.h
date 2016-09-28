#ifndef IMAGECONF_H
#define IMAGECONF_H
#include <string>
#if 0
/* picture name */
extern const std::string BACKGROUND_IMG = "background.png";
extern const std::string FRONVIEW_CONFIGURE_TITLE_IMG = "Fronview_configure_title.png";
extern const std::string FRONVIEW_LOGIN_BACKGROUND_IMG = "fronview_login_background.png";
//general
extern const std::string ABOUT_VCL_LOGO_IMG = "about_vCl_logo.png";
extern const std::string CORNER_LEFT_TOP_IMG = "corner_left_top.png";
extern const std::string FIRSTLOADING_IMG = "firstloading.gif";
extern const std::string VERTICAL_LINE_IMG = "vertical_line.png";
//jsjq
extern const std::string JSJQ_TITLE_IMG = "jsjq_title.png";
extern const std::string LOGO_JSJQ_IMG = "logo_jsjq.png";

//version dir
extern const std::string VERSION_FILE_DIR = "/etc/fronview-version";
extern const std::string IMAG_DIR = "/usr/share/images/";
extern const std::string GENERAL_DIR = "/usr/share/images/index/";
extern const std::string JSJQ_DIR = "/usr/share/images/jsjq/";
extern const std::string WINDOWS_IMG = "/usr/share/images/index/about_vCl_logo.png";
#endif

extern const std::string BACKGROUND_IMG;
extern const std::string FRONVIEW_CONFIGURE_TITLE_IMG;
extern const std::string FRONVIEW_LOGIN_BACKGROUND_IMG;
//general
extern const std::string ABOUT_VCL_LOGO_IMG;
extern const std::string CORNER_LEFT_TOP_IMG;
extern const std::string FIRSTLOADING_IMG;
extern const std::string VERTICAL_LINE_IMG;
//jsjq
extern const std::string JSJQ_TITLE_IMG;
extern const std::string LOGO_JSJQ_IMG;
extern const std::string ICON_CLOSE;
extern const std::string ICON_MINIMIZE;

//version dir
extern const std::string VERSION_FILE_DIR;
extern const std::string IMAG_DIR;
extern const std::string GENERAL_DIR;
extern const std::string JSJQ_DIR;
extern const std::string WINDOWS_IMG;
extern const std::string GENERAL_VCLASS_DIR;
extern const std::string STYLE_SHEET_TEXT;

typedef struct image
{
    std::string background;
    std::string Fronview_configure_title;
    std::string fronview_login_background;
    std::string about_vCl_logo;
    std::string corner_left_top;
    std::string firstloading;
    std::string vertical_line;
    std::string jsjq_title;
    std::string logo_jsjq;
    std::string icon_close;
    std::string icon_minimize;
}IMAGE_S;
extern IMAGE_S vclient_image;

#endif // IMAGECONF_H
