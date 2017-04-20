/*
C++ 11的线程管理
后台线程的使用场景示例
*/

#include <thread>
#include <string>
#include <iostream>

static void edit_document(const std::string& filename)
{
  open_document_and_display_gui(filename);
  while(!done_editing())
  {
    user_command cmd=get_user_input();
    if(cmd.type==open_new_document)
    {
      const std::string new_name=get_filename_from_user();
      std::thread t(edit_document,new_name);  //复用edit_document函数打开并编辑一个新文档，并传入新文件名
      t.detach();  // 必须要分离
    }
    else
    {
       process_user_input(cmd);
    }
  }
}

int main()
{
    edit_document("default-doc.docx");
    return 0;
}