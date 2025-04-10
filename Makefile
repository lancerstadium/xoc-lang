.PHONY: all run clean install uninstall commit stats stats-diff


# -- Installation Prefix
APP_NAME 		?= xoc
PLATFORM 		?= $(shell uname -s)
PREFIX 			?= /usr/local
TRG_DIR 		?= build
OBJ_DIR 		?= obj
SRC_DIR 		?= src
INC_DIR 		?= include
LIB_DIR 		?= lib
BIN_DIR 		?= bin
TEST_DIR 		?= test
CUR_TIME 		?= $(shell date +%Y-%m-%d_%H-%M-%S)
APP_API 		?= $(INC_DIR)/$(APP_NAME)_api.h
SRCS 			= $(filter-out src/$(APP_NAME).c,$(wildcard src/*.c))
OBJS_STATIC 	= $(sort $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%_s.o))
OBJS_DYNAMIC 	= $(sort $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%_d.o))
OBJS_EXE		= $(OBJ_DIR)/$(APP_NAME)_s.o

# -- Global Tool Settings
CROSS_COMPILE 	?=
CC 				?= $(CROSS_COMPILE)gcc
CXX 			?= $(CROSS_COMPILE)g++
LD 				?= $(CROSS_COMPILE)ld
RM 				?= rm

# -- Platform Specific Settings:
ifeq ($(PLATFORM),Linux)
	LDFLAGS               = -lm -ldl
	RANLIB                = ar -crs
	LIBEXT                = so
	DYNAMIC_CFLAGS_EXTRA  = -shared -fvisibility=hidden
else ifeq ($(PLATFORM), Darwin)
	LDFLAGS               =
	RANLIB                = libtool -static -o
	LIBEXT                = dylib
	DYNAMIC_CFLAGS_EXTRA  = -dynamiclib -fvisibility=hidden
else ifneq ($(findstring MINGW64_NT,$(PLATFORM)),)
	LDFLAGS               = -lm
	RANLIB                = ar -crs
	LIBEXT                = so
	DYNAMIC_CFLAGS_EXTRA  = -shared -fvisibility=hidden
endif

# -- Identical for All Platforms
APP_SLIB        = $(TRG_DIR)/lib$(APP_NAME).a
APP_DLIB        = $(TRG_DIR)/lib$(APP_NAME).$(LIBEXT)
APP_EXE		 	= $(TRG_DIR)/$(APP_NAME)

CFLAGS 			= -s -fPIC -O3 -Wall -Wno-format-security -malign-double -fno-strict-aliasing -I$(INC_DIR)
C_SFLGS 		= $(CFLAGS) -D$(APP_NAME)_BUILD_STATIC
C_DFLGS 		= $(CFLAGS) -D$(APP_NAME)_BUILD_DYNAMIC $(DYNAMIC_CFLAGS_EXTRA)

# -- Applications
all:			$(APP_EXE) $(APP_SLIB) $(APP_DLIB)
static:			$(APP_SLIB)
dynamic: 		$(APP_DLIB)
exe: 			$(APP_EXE)

run:			$(APP_EXE)
	@./$(APP_EXE)

clean:
	$(RM) -r $(TRG_DIR) $(OBJ_DIR)

install: all
	@echo "Installing `$(APP_NAME)` to $(PREFIX): "
	@mkdir -p $(PREFIX)/$(include)
	@mkdir -p $(PREFIX)/$(lib)
	@mkdir -p $(PREFIX)/$(bin)
	@echo " -- $(APP_SLIB)"
	@echo " -- $(APP_DLIB)"
	@echo " -- $(APP_EXE)"
	@echo " -- $(APP_API)"
	@cp -f $(APP_SLIB) $(PREFIX)/$(lib)
	@cp -f $(APP_DLIB) $(PREFIX)/$(lib)
	@cp -f $(APP_EXE) $(PREFIX)/$(bin)
	@cp -f $(APP_API) $(PREFIX)/$(include)
	@echo "Installation complete."

uninstall:
	@echo "Uninstalling `$(APP_NAME)` from $(PREFIX): "
	@echo " -- $(APP_SLIB)"
	@echo " -- $(APP_DLIB)"
	@echo " -- $(APP_EXE)"
	@echo " -- $(APP_API)"
	@$(RM) -f $(PREFIX)/$(lib)/lib$(APP_NAME).a
	@$(RM) -f $(PREFIX)/$(lib)/lib$(APP_NAME).$(LIBEXT)
	@$(RM) -f $(PREFIX)/$(bin)/$(APP_NAME)
	@$(RM) -f $(PREFIX)/$(include)/$(APP_NAME)_api.h
	@echo "Uninstallation complete."

commit: stats
	@git add .
	@git commit -m "$(CUR_TIME)"
	@git push

stats:
	@echo "Generating code statistics..."
	@mkdir -p .stats
	@if [ -f .stats/last.txt ]; then \
	  make -s stats-diff; \
	  cp .stats/current.txt .stats/last.txt; \
	else \
	  cp .stats/current.txt .stats/last.txt; \
	  echo "Initial stats saved to .stats/last.txt"; \
	fi

stats-diff:
	@find $(SRC_DIR) $(INC_DIR) -type f \( -name "*.c" -o -name "*.h" \) -exec wc -l {} + | \
	  awk '$$2 != "total" {print $$2,$$1}' | sort > .stats/current.txt
	@echo "+---------------------------------+---------+---------+-------------+"
	@echo "| File                            |   Old   |   New   |   Changes   |"
	@echo "|---------------------------------+---------|---------+-------------|"
	@join -a1 -a2 -e0 -o 0,1.2,2.2 .stats/last.txt .stats/current.txt | \
	awk ' \
	  { \
	    file=$$1; old=$$2; new=$$3; \
	    if (old == 0) { \
	      printf "| %-31s | %7s | %7d | \033[32m+%10d\033[0m |\n", file, "--", new, new; \
	      added+=new; \
	    } else if (new == 0) { \
	      printf "| %-31s | %7d | %7s | \033[31m-%10d\033[0m |\n", file, old, "--", old; \
	      removed+=old; \
	    } else if (old != new) { \
	      diff=new-old; \
	      if (diff > 0) { \
	        printf "| %-31s | %7d | %7d | \033[32m+%10d\033[0m |\n", file, old, new, diff; \
	        added+=diff; \
	      } else { \
	        printf "| %-31s | %7d | %7d | \033[31m-%10d\033[0m |\n", file, old, new, -diff; \
	        removed-=diff; \
	      } \
	    } else { \
	      printf "| %-31s | %7d | %7d | %11s |\n", file, old, new, "--"; \
	    } \
	    total_new+=new; \
	    total_old+=old; \
	  } \
	  END { \
	    printf "|---------------------------------+---------+---------+-------------|\n"; \
	    printf "| %-31s | %7d | %7d | \033[33m%+11d\033[0m |\n", "TOTAL", total_old, total_new, total_new-total_old; \
	    printf "+---------------------------------+---------+---------+-------------+\n"; \
	  }'

$(APP_SLIB): $(OBJS_STATIC)
	@echo "[AR] $@"
	@mkdir -p $(TRG_DIR)/$(INC_DIR)
	@$(RANLIB) $@ $^
	@cp $(APP_API) $(TRG_DIR)/$(INC_DIR)

$(APP_DLIB): $(OBJS_DYNAMIC)
	@echo "[LD] $@"
	@mkdir -p $(TRG_DIR)/$(INC_DIR)
	@$(CC) $(C_DFLGS) -o $@ $^ $(LDFLAGS)
	@cp $(APP_API) $(TRG_DIR)/$(INC_DIR)

$(APP_EXE): $(OBJS_EXE) $(APP_SLIB)
	@echo "[LD] $@"
	@mkdir -p $(dir $@)
	@$(CC) $(C_SFLGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%_s.o: $(SRC_DIR)/%.c
	@echo "[CC] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(C_SFLGS) -o $@ -c $<

$(OBJ_DIR)/%_d.o: $(SRC_DIR)/%.c
	@echo "[CC] $<"
	@mkdir -p $(dir $@)
	@$(CC) $(C_DFLGS) -o $@ -c $<