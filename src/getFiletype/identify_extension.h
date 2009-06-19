
#ifndef IDENTIFY_EXTENSION_H
#define IDENTIFY_EXTENSION_H


struct fte_data
{
    int		lang_size, group_size, descr_size, ext_size;
    char	**lang, **ext, **version, **ext_sorted, **ext_icon, **group_icon;
    int		*ext2descr, *ext2group;
    char	***group, ***descr;
    char	**default_group, **default_descr;
    char	*default_icon;
    char	_default_descr_array_[32];
};


struct fte_data* fte_init( char *conf_file );
void fte_destroy(struct fte_data *fdata);
char* fte_getdefaultgroup(struct fte_data *fdata, char *lang, char **icon);
char fte_belongs_to_group(struct fte_data *fdata, char *lang, char *ext, char *group);
int fte_getdescription(struct fte_data *fdata, char *lang, char *ext, char **group, char **descr, char **icon, char **version);
int fte_getextension(struct fte_data *fdata, char *lang, char *group, char ***ptr1, char ***ptr2);
int fte_getext_from_ext(struct fte_data *fdata, char *ext, char ***ptr1, char ***ptr2);
int fte_groupid(struct fte_data *fdata, char *lang, char *group, char **icon);
int fte_extid(struct fte_data *fdata, char *ext);

#endif	// IDENTIFY_EXTENSION_H
