
objdir:
	if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi

.PHONY:	objdir
