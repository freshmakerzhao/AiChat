#include <utils/markdownUtil.h>
#include <QByteArray>
extern "C" {
	#include <md4c-html.h> 
}

namespace {
	static void md4c_write_cb(const MD_CHAR* data,MD_SIZE size,void* userdata) {
		auto* out = static_cast<QByteArray*>(userdata);
		out->append(reinterpret_cast<const char*>(data),
					static_cast<int>(size));
	}
} // namespace

namespace MarkdownUtil {

    QString markdownToHtml(const QString& md, int basePointSize, bool disableHtml) {
		const QByteArray input = md.toUtf8();

		unsigned md_flags =
			MD_FLAG_TABLES |
			MD_FLAG_TASKLISTS |
			MD_FLAG_STRIKETHROUGH |
			MD_FLAG_PERMISSIVEURLAUTOLINKS |
			MD_FLAG_PERMISSIVEEMAILAUTOLINKS |
			MD_FLAG_PERMISSIVEWWWAUTOLINKS;
		
		if(disableHtml) {
			md_flags |= MD_FLAG_NOHTMLSPANS | MD_FLAG_NOHTMLBLOCKS;
		}

		unsigned html_flags = 0;

		QByteArray output;
		output.reserve(input.size() * 2);

		const int rc = md_html(
			input.constData(),
			static_cast<MD_SIZE>(input.size()),
			md4c_write_cb,
			&output,
			md_flags,
			html_flags
		);

		if(rc != 0) {
			return md; 
		}

		const QString html = QString::fromUtf8(output);

		return QString::fromLatin1(
			"<html><head><meta charset='utf-8'>"
			"<style>"
			"body{font-size:%1pt; line-height:1.5;}"
			"pre,code{font-family:Consolas,'Fira Code',monospace;}"
			"ul,ol{margin:0 0 0 1.2em;}"
			"p{margin:0.4em 0;}"
			"blockquote{margin:0.4em 0; padding:0.4em 0.8em; border-left:3px solid #ccc; background:#f7f7f7;}"
			"hr{border:none; border-top:1px solid #ddd; margin:0.6em 0;}"
			"table{border-collapse:collapse;}"
			"th,td{border:1px solid #ddd; padding:4px 6px;}"
			"</style></head><body>%2</body></html>"
		).arg(basePointSize).arg(html);

    }

} // namespace MarkdownUtil
