ARDUINO_ALL_LIBRARIES := $(patsubst $(COMPONENT_PATH)/%,%,$(wildcard $(COMPONENT_PATH)/*))

# Macro returns non-empty if Arduino library $(1) should be included in the build
# (either because selective compilation is of, or this library is enabled
define ARDUINO_LIBRARY_ENABLED
$(if $(CONFIG_ARDUINO_SELECTIVE_COMPILATION),$(CONFIG_ARDUINO_SELECTIVE_$(1)),y)
endef

ARDUINO_ENABLED_LIBRARIES := $(foreach LIBRARY,$(sort $(ARDUINO_ALL_LIBRARIES)),$(if $(call ARDUINO_LIBRARY_ENABLED,$(LIBRARY)),$(LIBRARY)))

$(info External Arduino libraries in build: $(ARDUINO_ENABLED_LIBRARIES))

# Expand all subdirs under $(1)
define EXPAND_SUBDIRS
$(sort $(dir $(wildcard $(1)/* $(1)/*/* $(1)/*/*/* $(1)/*/*/*/* $(1)/*/*/*/*/*)))
endef

# Macro returns SRCDIRS for library
define ARDUINO_LIBRARY_GET_SRCDIRS
	$(if $(wildcard $(COMPONENT_PATH)/$(1)/src/.), 							\
		$(call EXPAND_SUBDIRS,$(COMPONENT_PATH)/$(1)/src), 					\
		$(filter-out $(call EXPAND_SUBDIRS,$(COMPONENT_PATH)/$(1)/examples),  \
			$(call EXPAND_SUBDIRS,$(COMPONENT_PATH)/$(1)) 				   	\
		) 																				\
	)
endef

# Macro returns LIBDIRS for library
define ARDUINO_LIBRARY_GET_LIBDIRS
	$(wildcard $(COMPONENT_PATH)/$(1)/src/esp32/*.a)
endef

# Make a list of all srcdirs in enabled libraries
ARDUINO_LIBRARY_SRCDIRS := $(patsubst $(COMPONENT_PATH)/%,%,$(foreach LIBRARY,$(ARDUINO_ENABLED_LIBRARIES),$(call ARDUINO_LIBRARY_GET_SRCDIRS,$(LIBRARY))))

ARDUINO_LIBRARY_PRECOMPILED := $(patsubst $(COMPONENT_PATH)/%,%,$(foreach LIBRARY,$(ARDUINO_ENABLED_LIBRARIES),$(call ARDUINO_LIBRARY_GET_LIBDIRS,$(LIBRARY))))
ARDUINO_LIBRARY_LDFLAGS := $(addprefix $(COMPONENT_PATH)/,$(ARDUINO_LIBRARY_PRECOMPILED))

$(info External precompiled Arduino libraries to link: $(ARDUINO_LIBRARY_PRECOMPILED))

COMPONENT_ADD_INCLUDEDIRS := $(ARDUINO_LIBRARY_SRCDIRS)
COMPONENT_ADD_LINKER_DEPS := $(ARDUINO_LIBRARY_PRECOMPILED)
COMPONENT_ADD_LDFLAGS += $(ARDUINO_LIBRARY_LDFLAGS)
COMPONENT_SRCDIRS := $(ARDUINO_LIBRARY_SRCDIRS)
