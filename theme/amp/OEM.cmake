set( APPLICATION_NAME       "AMPcare" )
set( APPLICATION_SHORTNAME  "AMPcare" )
set( APPLICATION_EXECUTABLE "ampcare" )
set( APPLICATION_DOMAIN     "amp.care" )
set( APPLICATION_VENDOR     "Awesome Technologies Innovationslabor GmbH" )
set( APPLICATION_UPDATE_URL "https://updates.amp.care/client/" CACHE string "URL for updater" )
set( APPLICATION_ICON_NAME  "AMP" )

set( LINUX_PACKAGE_SHORTNAME "ampcare" )

set( THEME_CLASS            "AMPTheme" )
set( APPLICATION_REV_DOMAIN "care.amp.desktopclient" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

set( THEME_INCLUDE          "${OEM_THEME_DIR}/amptheme.h" )
# set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt )

option( WITH_CRASHREPORTER "Build crashreporter" OFF )
#set( CRASHREPORTER_SUBMIT_URL "https://crash-reports.amp.care/submit" CACHE string "URL for crash reporter" )
#set( CRASHREPORTER_ICON ":/amp-icon.png" )

## Theming options
set( APPLICATION_WIZARD_HEADER_BACKGROUND_COLOR "#0C023B" CACHE string "Hex color of the wizard header background")
set( APPLICATION_WIZARD_HEADER_TITLE_COLOR "#ffffff" CACHE string "Hex color of the text in the wizard header")
