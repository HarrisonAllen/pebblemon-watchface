AR = 'arm-none-eabi-ar'
ARFLAGS = 'rcs'
AS = 'arm-none-eabi-gcc'
BINDIR = '/usr/local/bin'
BLOCK_MESSAGE_KEYS = []
BUILD_DIR = 'diorite'
BUILD_TYPE = 'app'
BUNDLE_BIN_DIR = 'diorite'
BUNDLE_NAME = 'pebblemon-watchface.pbw'
CC = ['arm-none-eabi-gcc']
CCLNK_SRC_F = []
CCLNK_TGT_F = ['-o']
CC_NAME = 'gcc'
CC_SRC_F = []
CC_TGT_F = ['-c', '-o']
CC_VERSION = ('4', '7', '2')
CFLAGS = ['-std=c99', '-mcpu=cortex-m3', '-mthumb', '-ffunction-sections', '-fdata-sections', '-g', '-fPIE', '-Os', '-D_TIME_H_', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter', '-Wno-error=unused-function', '-Wno-error=unused-variable']
CFLAGS_MACBUNDLE = ['-fPIC']
CFLAGS_cshlib = ['-fPIC']
CPPPATH_ST = '-I%s'
DEFINES = ['RELEASE', 'PBL_PLATFORM_DIORITE', 'PBL_BW', 'PBL_RECT', 'PBL_MICROPHONE', 'PBL_HEALTH', 'PBL_SMARTSTRAP', 'PBL_DISPLAY_WIDTH=144', 'PBL_DISPLAY_HEIGHT=168', 'PBL_SDK_3']
DEFINES_ST = '-D%s'
DEST_BINFMT = 'elf'
DEST_CPU = 'arm'
DEST_OS = 'linux'
INCLUDES = ['diorite']
LD = 'arm-none-eabi-ld'
LIBDIR = '/usr/local/lib'
LIBPATH_ST = '-L%s'
LIB_DIR = 'node_modules'
LIB_JSON = []
LIB_ST = '-l%s'
LINKFLAGS = ['-mcpu=cortex-m3', '-mthumb', '-Wl,--gc-sections', '-Wl,--warn-common', '-fPIE', '-Os']
LINKFLAGS_MACBUNDLE = ['-bundle', '-undefined', 'dynamic_lookup']
LINKFLAGS_cshlib = ['-shared']
LINKFLAGS_cstlib = ['-Wl,-Bstatic']
LINK_CC = ['arm-none-eabi-gcc']
MESSAGE_KEYS = {u'dummy': 10000}
MESSAGE_KEYS_DEFINITION = '/mnt/c/Users/harri/Documents/pebble/pebblemon-watchface/build/src/message_keys.auto.c'
MESSAGE_KEYS_HEADER = '/mnt/c/Users/harri/Documents/pebble/pebblemon-watchface/build/include/message_keys.auto.h'
MESSAGE_KEYS_JSON = '/mnt/c/Users/harri/Documents/pebble/pebblemon-watchface/build/js/message_keys.json'
NODE_PATH = '/home/linksky/.pebble-sdk/SDKs/current/node_modules'
PEBBLE_SDK_COMMON = '/home/linksky/.pebble-sdk/SDKs/current/sdk-core/pebble/common'
PEBBLE_SDK_PLATFORM = '/home/linksky/.pebble-sdk/SDKs/current/sdk-core/pebble/diorite'
PEBBLE_SDK_ROOT = '/home/linksky/.pebble-sdk/SDKs/current/sdk-core/pebble'
PLATFORM = {'TAGS': ['diorite', 'bw', 'rect', 'mic', 'strap', 'health', '144w', '168h'], 'MAX_FONT_GLYPH_SIZE': 256, 'ADDITIONAL_TEXT_LINES_FOR_PEBBLE_H': [], 'MAX_APP_BINARY_SIZE': 65536, 'MAX_RESOURCES_SIZE': 1048576, 'MAX_APP_MEMORY_SIZE': 65536, 'MAX_WORKER_MEMORY_SIZE': 10240, 'NAME': 'diorite', 'BUNDLE_BIN_DIR': 'diorite', 'BUILD_DIR': 'diorite', 'MAX_RESOURCES_SIZE_APPSTORE': 262144, 'DEFINES': ['PBL_PLATFORM_DIORITE', 'PBL_BW', 'PBL_RECT', 'PBL_MICROPHONE', 'PBL_HEALTH', 'PBL_SMARTSTRAP', 'PBL_DISPLAY_WIDTH=144', 'PBL_DISPLAY_HEIGHT=168']}
PLATFORM_NAME = 'diorite'
PREFIX = '/usr/local'
PROJECT_INFO = {'appKeys': {u'dummy': 10000}, u'watchapp': {u'watchface': True}, u'displayName': u'Pebblemon Time', u'uuid': u'2b5ff5d9-75a2-4f8e-866d-015216a3e45c', u'messageKeys': {u'dummy': 10000}, 'companyName': u'Harrison Allen', u'enableMultiJS': True, u'sdkVersion': u'3', 'versionLabel': u'1.0', u'targetPlatforms': [u'aplite', u'basalt', u'chalk', u'diorite'], 'longName': u'Pebblemon Time', 'shortName': u'Pebblemon Time', u'resources': {u'media': [{u'menuIcon': True, u'type': u'png', u'name': u'IMAGE_MENU_ICON', u'file': u'menu_icon.png'}, {u'type': u'raw', u'name': u'DATA_ANIMATION_TILESHEET', u'file': u'data/AnimationTilesheet.bin'}, {u'type': u'raw', u'name': u'DATA_WORLD_TILESHEET', u'file': u'data/WorldTilesheet.bin'}, {u'type': u'raw', u'name': u'DATA_CAVE', u'file': u'data/Cave.bin'}, {u'type': u'raw', u'name': u'DATA_FOREST', u'file': u'data/Forest.bin'}, {u'type': u'raw', u'name': u'DATA_NATIONAL_PARK', u'file': u'data/NationalPark.bin'}, {u'type': u'raw', u'name': u'DATA_ROUTE_1', u'file': u'data/Route1.bin'}, {u'type': u'raw', u'name': u'DATA_ROUTE_2', u'file': u'data/Route2.bin'}, {u'type': u'raw', u'name': u'DATA_SPRITESHEET', u'file': u'data/Spritesheet.bin'}, {u'type': u'raw', u'name': u'DATA_PATH', u'file': u'data/Path.bin'}]}, 'name': u'pebblemon-watchface'}
REQUESTED_PLATFORMS = [u'aplite', u'basalt', u'chalk', u'diorite']
RESOURCES_JSON = [{u'menuIcon': True, u'type': u'png', u'name': u'IMAGE_MENU_ICON', u'file': u'menu_icon.png'}, {u'type': u'raw', u'name': u'DATA_ANIMATION_TILESHEET', u'file': u'data/AnimationTilesheet.bin'}, {u'type': u'raw', u'name': u'DATA_WORLD_TILESHEET', u'file': u'data/WorldTilesheet.bin'}, {u'type': u'raw', u'name': u'DATA_CAVE', u'file': u'data/Cave.bin'}, {u'type': u'raw', u'name': u'DATA_FOREST', u'file': u'data/Forest.bin'}, {u'type': u'raw', u'name': u'DATA_NATIONAL_PARK', u'file': u'data/NationalPark.bin'}, {u'type': u'raw', u'name': u'DATA_ROUTE_1', u'file': u'data/Route1.bin'}, {u'type': u'raw', u'name': u'DATA_ROUTE_2', u'file': u'data/Route2.bin'}, {u'type': u'raw', u'name': u'DATA_SPRITESHEET', u'file': u'data/Spritesheet.bin'}, {u'type': u'raw', u'name': u'DATA_PATH', u'file': u'data/Path.bin'}]
RPATH_ST = '-Wl,-rpath,%s'
SANDBOX = False
SDK_VERSION_MAJOR = 5
SDK_VERSION_MINOR = 86
SHLIB_MARKER = None
SIZE = 'arm-none-eabi-size'
SONAME_ST = '-Wl,-h,%s'
STLIBPATH_ST = '-L%s'
STLIB_MARKER = None
STLIB_ST = '-l%s'
SUPPORTED_PLATFORMS = ['aplite', 'basalt', 'chalk', 'diorite', 'emery']
TARGET_PLATFORMS = ['diorite', 'chalk', 'basalt', 'aplite']
TIMESTAMP = 1626231162
USE_GROUPS = True
VERBOSE = 0
WEBPACK = '/home/linksky/.pebble-sdk/SDKs/current/node_modules/.bin/webpack'
cprogram_PATTERN = '%s'
cshlib_PATTERN = 'lib%s.so'
cstlib_PATTERN = 'lib%s.a'
macbundle_PATTERN = '%s.bundle'
