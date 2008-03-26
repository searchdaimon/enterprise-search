BEGIN   { size = 0; }
        { A[size] = $1; B[size] = $2; size++; }
END     {
	    printf("// Generert av tags.awk, (C) 2007 Boitho AS, Magnus Galåen.\n");
	    printf("// Forandringer til denne fila vil bli overskrevet, se tags.conf og tags.awk istedet.\n\n");
	    printf("\n#ifndef _HTML_TAGS_H_\n#define _HTML_TAGS_H_\n\n\n");
	    printf("#define tagf_space	1\n");
	    printf("#define tagf_head	2\n");
	    printf("#define tagf_div	4\n");
	    printf("#define tagf_span	8\n");
	    printf("#define tagf_block	16\n");
	    printf("#define tagf_inline	32\n");
	    printf("#define tagf_empty	64\n\n");

            printf("char*		tags[] = {");
            for (i=0; i<size; i++)
                {
                    printf("\"%s\"", A[i]);
                    if (i<size-1)
                        printf(", ");
                }
            printf("};\n");

            printf("enum		{");
            for (i=0; i<size; i++)
                {
                    printf("tag_%s", A[i]);
                    if (i==0)
                        printf("=0");
                    if (i<size-1)
                        printf(", ");
                }
            printf("};\n");

            printf("const int	tag_flags[] = {");
            for (i=0; i<size; i++)
                {
                    printf("%s", B[i]);
                    if (i<size-1)
                        printf(", ");
                }
            printf("};\n");

	    printf("const int	tags_size = %i;\n", size);
	    printf("automaton	*tags_automaton = NULL;\n");
	    printf("\n\n#endif\t// _HTML_TAGS_H_\n");
        }
