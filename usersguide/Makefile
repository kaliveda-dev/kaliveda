MKDWN_EXT = .txt
#GLOBAL_PANDOC_OPTIONS = --mathjax --number-sections --pdf-engine=pdflatex -s --toc --toc-depth=5 -c css/rootdoc.css --section-divs
GLOBAL_PANDOC_OPTIONS = --number-sections --pdf-engine=pdflatex -s --toc --toc-depth=5 -c css/rootdoc.css --section-divs
MKDWN_SOURCES := $(wildcard *$(MKDWN_EXT))
#GLOBAL_PANDOC_OPTIONS = --mathjax --number-sections --pdf-engine=pdflatex -s --toc --toc-depth=5 -c rootdoc.css --section-divs
#MKDWN_SOURCES := indrafazia-e789.txt

HTML_OUTPUT := $(patsubst %$(MKDWN_EXT),html/%.html,$(MKDWN_SOURCES))
PDF_OUTPUT  := $(patsubst %$(MKDWN_EXT),pdf/%.pdf,$(MKDWN_SOURCES))
TEX_OUTPUT  := $(patsubst %$(MKDWN_EXT),tex/%.tex,$(MKDWN_SOURCES))

OUTPUT = $(HTML_OUTPUT)

.PHONY: all clean html pdf tex links_to_doc install

all: links_to_doc

html: $(HTML_OUTPUT)
pdf: $(PDF_OUTPUT)
tex: $(TEX_OUTPUT)

clean:
	@rm -f $(OUTPUT) userguide_TOC.html
	
.SUFFIXES: $(MKDWN_EXT) .html .pdf .tex

html/%.html: %$(MKDWN_EXT)
	@echo "Generating $@ ..."
	@mkdir -p html
	@pandoc $(GLOBAL_PANDOC_OPTIONS) -f markdown+tex_math_dollars -t html4 -s -o $@ $< 

pdf/%.pdf: %$(MKDWN_EXT)
	@echo "Generating $@ ..."
	@mkdir -p pdf
	@pandoc $(GLOBAL_PANDOC_OPTIONS) -f markdown -t pdf -s -o $@ $< 

tex/%.tex: %$(MKDWN_EXT)
	@echo "Generating $@ ..."
	@mkdir -p tex
	@pandoc $(GLOBAL_PANDOC_OPTIONS) -f markdown -t latex -s -o $@ $< 

# find and strip main title in markdown
# sed -n '/^# /p' filter.txt | sed 's/^[# ]*// ; s/[ #]*$//'
links_to_doc:
	@mkdir -p html
	@echo \<ol\> > userguide_TOC.html ; \
        i=1;\
	for docfile in $(MKDWN_SOURCES); do \
		title=`sed -n '/ #$$/p' $$docfile | sed -n '/^# /p' | sed 's/^[# ]*// ; s/[ #]*$$//'`; \
		echo \<li\>\<a href=\"UsersGuide\/$${docfile%.*}.html\"\>$$title\<\/a\>\<\/li\> >> userguide_TOC.html ; \
		htmlfile="html/`basename --suffix=$(MKDWN_EXT) $$docfile`.html";\
                pandoc $(GLOBAL_PANDOC_OPTIONS) --number-offset=$$((i-1)) -f markdown -t html -s -o $$htmlfile $$docfile;\
                i=$$((i+1));\
	done ; \
	echo \<\/ol\> >> userguide_TOC.html

install:
	@mkdir -p $(PREFIX)/UsersGuide
	@mkdir -p $(PREFIX)/UsersGuide/css
	@cp userguide_TOC.html ../
	@cp $(HTML_OUTPUT) $(PREFIX)/UsersGuide/
	@cp rootdoc.css $(PREFIX)/UsersGuide/css
