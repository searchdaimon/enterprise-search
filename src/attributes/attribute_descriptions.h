
#ifndef ATTR_DESCR_H
#define ATTR_DESCR_H


struct adf_data
{
    int		lang_size, keys_size, values_size;
    int		*keys2values;
    char	**lang, **keys_icon, **values_icon;
    char	**key_attr, **value_attr;
    char	***keys, ***values;
};


struct adf_data* adf_init( char *conf_file );
void adf_destroy(struct adf_data *adata);
int adf_get_val_descr(struct adf_data *adata, char *lang, char *key, char *value, char **description, char **icon);
int adf_get_key_descr(struct adf_data *adata, char *lang, char *key, char **description, char **icon);

#endif	// ATTR_DESCR_H
