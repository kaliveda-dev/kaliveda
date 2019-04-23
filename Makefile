#Makefile for KaliVeda web site/documentation

###########################################
#                                         #
#   DO NOT CHANGE ANY OF THE FOLLOWING    #
#                                         #
###########################################

prefix = $(HOME)/software/kaliveda.website


WEBSITE_URL = http://indra.in2p3.fr/kaliveda
WEBSITE_ROOT_DIR = htdocs/kaliveda
WEBSITE_UPLOAD = indra@indra.in2p3.fr:$(WEBSITE_ROOT_DIR)

.PHONY : clean current default upload_current update_all upload_all update_and_upload_all

lyxdocdirs=$(shell ls $(prefix)/KaliVedaDoc | grep Doc)
	
upload_main_site:
	@echo "Upload main website docs"
	@echo "put html/appli.html" > upload.batch
	@echo "put html/build.html" >> upload.batch
	@echo "put html/index.html" >> upload.batch
	@echo "put html/download.html" >> upload.batch
	@echo "mkdir css" >> upload.batch
	@echo "put css/* css/" >> upload.batch
	@echo "mkdir images" >> upload.batch
	@echo "put images/* images/" >> upload.batch
	for docdir in $(lyxdocdirs); do \
	  echo "mkdir $$docdir" >> upload.batch ; \
	  echo "put $(prefix)/KaliVedaDoc/$$docdir/* $$docdir/" >> upload.batch ; \
	done
	@cat upload.batch
	sftp $(WEBSITE_UPLOAD) < upload.batch
	
update_main_site:
	@echo "Uploading updated main website docs"
	@echo "put html/appli.html" > upload.batch
	@echo "put html/build.html" >> upload.batch
	@echo "put html/index.html" >> upload.batch
	@echo "put html/download.html" >> upload.batch
	@echo "put css/* css/" >> upload.batch
	@echo "put images/* images/" >> upload.batch
	for docdir in $(lyxdocdirs); do \
	  echo "put $(prefix)/KaliVedaDoc/$$docdir/* $$docdir/" >> upload.batch ; \
	done
	@cat upload.batch
	sftp $(WEBSITE_UPLOAD) < upload.batch
	
main_site:
	@echo "Updating main website docs"
	@-mkdir -p $(prefix)/KaliVedaDoc
	@cd LyXGenDoc && $(MAKE) clean
	@cd LyXGenDoc && $(MAKE)
	@cd LyXGenDoc && $(MAKE) install PREFIX=$(prefix)/KaliVedaDoc
	@cat html/header.html html/appli.html html/tail.html > $(prefix)/KaliVedaDoc/appli.html
	@cat html/header.html html/build.html html/tail.html > $(prefix)/KaliVedaDoc/build.html
	@cat html/header.html html/download.html html/tail.html > $(prefix)/KaliVedaDoc/download.html
	@cat html/header.html html/index.html html/tail.html > $(prefix)/KaliVedaDoc/index.html
	@cp -r css $(prefix)/KaliVedaDoc/
	@cp -r js $(prefix)/KaliVedaDoc/
	@cp -r images $(prefix)/KaliVedaDoc/
	
upload_users_guide:
	@echo "Uploading users guide"
	@echo "mkdir UsersGuide" > upload.batch
	@echo "put $(prefix)/KaliVedaDoc/UsersGuide/* UsersGuide/" >> upload.batch
	@echo "mkdir UsersGuide/css" >> upload.batch
	@echo "put $(prefix)/KaliVedaDoc/UsersGuide/css/* UsersGuide/css/" >> upload.batch
	@echo "put $(prefix)/KaliVedaDoc/usersguide.html" >> upload.batch
	@cat upload.batch
	sftp $(WEBSITE_UPLOAD) < upload.batch
	
update_users_guide:
	@echo "Uploading updated users guide"
	@echo "put $(prefix)/KaliVedaDoc/usersguide.html" > upload.batch
	@echo "put $(prefix)/KaliVedaDoc/UsersGuide/* UsersGuide/" >> upload.batch
	@echo "put $(prefix)/KaliVedaDoc/UsersGuide/css/* UsersGuide/css/" >> upload.batch
	@cat upload.batch
	sftp $(WEBSITE_UPLOAD) < upload.batch
	
users_guide:
	@echo "Updating users guide"
	@cd usersguide && $(MAKE) && $(MAKE) install PREFIX=$(prefix)/KaliVedaDoc
	@cat html/header.html html/usersguide_head.html userguide_TOC.html html/tail.html > $(prefix)/KaliVedaDoc/usersguide.html

update_all: main_site users_guide
upload_all: update_main_site update_users_guide
update_and_upload_all: update_all upload_all
clean:
	@cd LyXGenDoc && $(MAKE) clean
	@cd usersguide && $(MAKE) clean
	@rm -f upload.batch userguide_TOC.html
        
distclean: clean
	-rm -rf KaliVedaDoc
 
