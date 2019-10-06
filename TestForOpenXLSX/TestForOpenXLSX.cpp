#include <stdio.h>
#pragma comment(lib,"OpenXLSX.lib")

int main(int argc, char* argv[])
{
	if (argc == 0)
		return -1;
	OpenXLSX::XLDocument doc;
	doc.OpenDocument(argv[1]);
	auto TargetWorkSheet = doc.Workbook().Worksheet(doc.Workbook().WorksheetNames()[0]);
	for (unsigned int i = 1; i <= TargetWorkSheet.ColumnCount(); i++)
	{
		printf("%s\n", TargetWorkSheet.Row(1).Cell(i).Value().AsString().c_str());
	}
	if (argc < 3)
		return 0;
	doc.SaveDocumentAs(std::string(argv[2]));
	if ()
}