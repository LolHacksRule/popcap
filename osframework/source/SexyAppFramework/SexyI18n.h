#ifndef SEXYI18N_H
#define SEXYI18N_H

#include <string>

namespace Sexy {

const char* tr(const char *s);
const std::string tr(const std::string &s);

const char* dtr(const char *domain, const char *s);
const std::string dtr(const std::string &domain, const std::string &s);

void setLocale(const char *locale);
void bindText(const char *domain, const char *dir);
void bindTextDomain(const char *domain, const char *dir);

}

#endif
