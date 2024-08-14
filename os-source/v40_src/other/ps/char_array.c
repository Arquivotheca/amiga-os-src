
#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "scanner.h"



UBYTE FLAGS[260] = {
(ISDELIM),				/* EOF  **NOTE** EOF is considered a delim in my code*/
(ISSPACE),			/* ESC 0 */
(ISSPACE),			/* ESC 1 */
(ISSPACE),			/* ESC 2 */
(ISSPACE),			/* ESC 3 */
(ISSPACE),			/* ESC 4 */
(ISSPACE),			/* ESC 5 */
(ISSPACE),			/* ESC 6 */
(ISSPACE),			/* ESC 7 */
(ISSPACE),			/* ESC 8 */
(ISSPACE),			/* ESC 9 */
(ISSPACE|ISESCAPE),			/* ESC 10 */
(ISSPACE),			/* ESC 11 */
(ISSPACE),			/* ESC 12 */
(ISSPACE),			/* ESC 13 */
(ISSPACE),			/* ESC 14 */
(ISSPACE),			/* ESC 15 */
(ISSPACE),			/* ESC 16 */
(ISSPACE),			/* ESC 17 */
(ISSPACE),			/* ESC 18 */
(ISSPACE),			/* ESC 19 */
(ISSPACE),			/* ESC 20 */
(ISSPACE),			/* ESC 21 */
(ISSPACE),			/* ESC 22 */
(ISSPACE),			/* ESC 23 */
(ISSPACE),			/* ESC 24 */
(ISSPACE),			/* ESC 25 */
(ISSPACE),			/* ESC 26 */
(ISSPACE),			/* ESC 27 */
(ISSPACE),			/* ESC 28 */
(ISSPACE),			/* ESC 29 */
(ISSPACE),			/* ESC 30 */
(ISSPACE),			/* ESC 31 */
(ISSPACE),			/*   */

(ISTOKEN),			/* ! */
(ISTOKEN),			/* " */
(ISTOKEN),			/* # */
(ISTOKEN),			/* $ */

(ISDELIM),			/* % */

(ISTOKEN),			/* & */
(ISTOKEN),			/* ' */

(ISDELIM|ISESCAPE),			/* ( */
(ISDELIM|ISESCAPE),			/* ) */

(ISTOKEN),			/* * */
(ISDIGIT),			/* + */
(ISTOKEN),			/* , */
(ISDIGIT),			/* - */
(ISTOKEN),			/* . */

(ISDELIM),			/* / */

(ISTOKEN|ISDIGIT|ISESCAPE),			/* 0 and also an octal digit */
(ISTOKEN|ISDIGIT|ISESCAPE),			/* 1 and also an octal digit */
(ISTOKEN|ISDIGIT|ISESCAPE),			/* 2 and also an octal digit */
(ISTOKEN|ISDIGIT|ISESCAPE),			/* 3 and also an octal digit */
(ISTOKEN|ISDIGIT|ISESCAPE),			/* 4 and also an octal digit */
(ISTOKEN|ISDIGIT|ISESCAPE),			/* 5 and also an octal digit */
(ISTOKEN|ISDIGIT|ISESCAPE),			/* 6 and also an octal digit */
(ISTOKEN|ISDIGIT|ISESCAPE),			/* 7 and also an octal digit */
(ISTOKEN|ISDIGIT),			/* 8 */
(ISTOKEN|ISDIGIT),			/* 9 */

(ISTOKEN),			/* : */
(ISTOKEN),			/* ; */

(ISDELIM),			/* < */

(ISTOKEN),			/* = */

(ISDELIM),			/* > */

(ISTOKEN),			/* ? */
(ISTOKEN),			/* @ */
(ISTOKEN),			/* A */
(ISTOKEN),			/* B */
(ISTOKEN),			/* C */
(ISTOKEN),			/* D */
(ISTOKEN),			/* E */
(ISTOKEN),			/* F */
(ISTOKEN),			/* G */
(ISTOKEN),			/* H */
(ISTOKEN),			/* I */
(ISTOKEN),			/* J */
(ISTOKEN),			/* K */
(ISTOKEN),			/* L */
(ISTOKEN),			/* M */
(ISTOKEN),			/* N */
(ISTOKEN),			/* O */
(ISTOKEN),			/* P */
(ISTOKEN),			/* Q */
(ISTOKEN),			/* R */
(ISTOKEN),			/* S */
(ISTOKEN),			/* T */
(ISTOKEN),			/* U */
(ISTOKEN),			/* V */
(ISTOKEN),			/* W */
(ISTOKEN),			/* X */
(ISTOKEN),			/* Y */
(ISTOKEN),			/* Z */

(ISDELIM),			/* [ */

(ISTOKEN),			/* \ */

(ISDELIM),			/* ] */

(ISTOKEN),			/* ^ */
(ISTOKEN),			/* _ */
(ISTOKEN),			/* ` */
(ISTOKEN),			/* a */
(ISTOKEN),			/* b */
(ISTOKEN),			/* c */
(ISTOKEN),			/* d */
(ISTOKEN),			/* e */
(ISTOKEN),			/* f */
(ISTOKEN),			/* g */
(ISTOKEN),			/* h */
(ISTOKEN),			/* i */
(ISTOKEN),			/* j */
(ISTOKEN),			/* k */
(ISTOKEN),			/* l */
(ISTOKEN),			/* m */
(ISTOKEN),			/* n */
(ISTOKEN),			/* o */
(ISTOKEN),			/* p */
(ISTOKEN),			/* q */
(ISTOKEN),			/* r */
(ISTOKEN),			/* s */
(ISTOKEN),			/* t */
(ISTOKEN),			/* u */
(ISTOKEN),			/* v */
(ISTOKEN),			/* w */
(ISTOKEN),			/* x */
(ISTOKEN),			/* y */
(ISTOKEN),			/* z */

(ISDELIM),			/* { */

(ISTOKEN),			/* | */

(ISDELIM),			/* } */

(ISTOKEN),			/* ~ */
(ISTOKEN),			/*   */

(ISDELIM|ISBINOBJ),			/* �  ASC 128 */
(ISDELIM|ISBINOBJ),			/* �  ASC 129 */
(ISDELIM|ISBINOBJ),			/* �  ASC 130 */
(ISDELIM|ISBINOBJ),			/* �  ASC 131 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 132 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 133 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 134 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 135 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 136 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 137 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 138 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 139 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 140 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 141 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 142 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 143 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 144 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 145 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 146 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 147 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 148 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 149 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 150 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 151 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 152 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 153 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 154 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 155 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 156 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 157 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 158 */
(ISDELIM|ISBINTOKEN),			/* �  ASC 159 */
(0),			/* �  ASC 160 */
(0),			/* �  ASC 161 */
(0),			/* �  ASC 162 */
(0),			/* �  ASC 163 */
(0),			/* �  ASC 164 */
(0),			/* �  ASC 165 */
(0),			/* �  ASC 166 */
(0),			/* �  ASC 167 */
(0),			/* �  ASC 168 */
(0),			/* �  ASC 169 */
(0),			/* �  ASC 170 */
(0),			/* �  ASC 171 */
(0),			/* �  ASC 172 */
(0),			/* �  ASC 173 */
(0),			/* �  ASC 174 */
(0),			/* �  ASC 175 */
(0),			/* �  ASC 176 */
(0),			/* �  ASC 177 */
(0),			/* �  ASC 178 */
(0),			/* �  ASC 179 */
(0),			/* �  ASC 180 */
(0),			/* �  ASC 181 */
(0),			/* �  ASC 182 */
(0),			/* �  ASC 183 */
(0),			/* �  ASC 184 */
(0),			/* �  ASC 185 */
(0),			/* �  ASC 186 */
(0),			/* �  ASC 187 */
(0),			/* �  ASC 188 */
(0),			/* �  ASC 189 */
(0),			/* �  ASC 190 */
(0),			/* �  ASC 191 */
(0),			/* �  ASC 192 */
(0),			/* �  ASC 193 */
(0),			/* �  ASC 194 */
(0),			/* �  ASC 195 */
(0),			/* �  ASC 196 */
(0),			/* �  ASC 197 */
(0),			/* �  ASC 198 */
(0),			/* �  ASC 199 */
(0),			/* �  ASC 200 */
(0),			/* �  ASC 201 */
(0),			/* �  ASC 202 */
(0),			/* �  ASC 203 */
(0),			/* �  ASC 204 */
(0),			/* �  ASC 205 */
(0),			/* �  ASC 206 */
(0),			/* �  ASC 207 */
(0),			/* �  ASC 208 */
(0),			/* �  ASC 209 */
(0),			/* �  ASC 210 */
(0),			/* �  ASC 211 */
(0),			/* �  ASC 212 */
(0),			/* �  ASC 213 */
(0),			/* �  ASC 214 */
(0),			/* �  ASC 215 */
(0),			/* �  ASC 216 */
(0),			/* �  ASC 217 */
(0),			/* �  ASC 218 */
(0),			/* �  ASC 219 */
(0),			/* �  ASC 220 */
(0),			/* �  ASC 221 */
(0),			/* �  ASC 222 */
(0),			/* �  ASC 223 */
(0),			/* �  ASC 224 */
(0),			/* �  ASC 225 */
(0),			/* �  ASC 226 */
(0),			/* �  ASC 227 */
(0),			/* �  ASC 228 */
(0),			/* �  ASC 229 */
(0),			/* �  ASC 230 */
(0),			/* �  ASC 231 */
(0),			/* �  ASC 232 */
(0),			/* �  ASC 233 */
(0),			/* �  ASC 234 */
(0),			/* �  ASC 235 */
(0),			/* �  ASC 236 */
(0),			/* �  ASC 237 */
(0),			/* �  ASC 238 */
(0),			/* �  ASC 239 */
(0),			/* �  ASC 240 */
(0),			/* �  ASC 241 */
(0),			/* �  ASC 242 */
(0),			/* �  ASC 243 */
(0),			/* �  ASC 244 */
(0),			/* �  ASC 245 */
(0),			/* �  ASC 246 */
(0),			/* �  ASC 247 */
(0),			/* �  ASC 248 */
(0),			/* �  ASC 249 */
(0),			/* �  ASC 250 */
(0),			/* �  ASC 251 */
(0),			/* �  ASC 252 */
(0),			/* �  ASC 253 */
(0),			/* �  ASC 254 */
(0),			/* �  ASC 255 */
0,0,0};/*PAD*/
