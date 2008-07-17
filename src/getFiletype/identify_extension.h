
#ifndef IDENTIFY_EXTENSION_H
#define IDENTIFY_EXTENSION_H


struct fte_data
{
    int		lang_size, group_size, descr_size, ext_size;
    char	**lang, **ext, **version;
    int		*ext2descr, *ext2group;
    char	***group, ***descr;
    char	**default_group, **default_descr;
    char	_default_descr_array_[32];
};


struct fte_data* fte_init( char *conf_file );
void fte_destroy(struct fte_data *fdata);
int fte_getdescription(struct fte_data *fdata, char *lang, char *ext, char **group, char **descr);
int fte_getextension(struct fte_data *fdata, char *lang, char *group, char ***ptr1, char ***ptr2);
int fte_getext_from_ext(struct fte_data *fdata, char *ext, char ***ptr1, char ***ptr2);

#endif	// IDENTIFY_EXTENSION_H
