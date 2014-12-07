include $(TOPDIR)/rules.mk

PKG_NAME:=noolite
PKG_VERSION:=0.7.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://github.com/olegart/noolite.git
PKG_SOURCE:=$(PKG_NAME).tar.gz
PKG_SOURCE_VERSION:=HEAD
PKG_SOURCE_SUBDIR:=$(PKG_NAME)

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/noolite
	SECTION:=base
	CATEGORY:=Utilities
	DEFAULT:=n
	DEPENDS:=+libusb-1.0
	TITLE:=nooLite smart home utilities
	#DESCRIPTION:=This variable is obsolete. use the Package/name/description define instead!
	URL:=http://olegart.ru
endef

define Package/noolite/description
	Utilities to control NooLite smart home USB receiver and transmitters
endef

define Build/Configure
	$(call Build/Configure/Default,--with-linux-headers=$(LINUX_DIR))
endef

define Package/noolite/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/noolitepc $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/nooliterx $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/nooliterxcfg $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/noolitepcd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,noolite))
