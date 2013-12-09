NAME=gawm

$(NAME).pdf:$(NAME).ps
	ps2pdf -dAutoRotatePages=/None $(NAME).ps

$(NAME).ps:$(NAME).dvi
	dvips -t a4 $(NAME).dvi

$(NAME).dvi: title.tex $(NAME).tex
	make -C img
#	vlna $(NAME).tex        # converts "v lese" to "v~lese" (viz: man vlna)
	latex -halt-on-error $(NAME).tex
	latex -halt-on-error $(NAME).tex	# Znovu pro vygenerování obsahu (seznamu sekcí)

open: clean $(NAME).pdf
	xdg-open $(NAME).pdf 2>/dev/null

clean:
	rm -f $(NAME).dvi $(NAME).aux $(NAME).toc $(NAME).log $(NAME).out
