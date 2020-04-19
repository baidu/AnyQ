'''
@Description: 读取Excel文件，生成FAQ文件
@Version: 1.0
@Author: Huyida
@Date: 2020-04-19 14:12:30
'''
import pandas as pd


def excel_one_line_to_list(path):
    """
    读取Exce文件，添加每行到list中
        path: 文件路径
        usecols： 读取列
    """
    # 读取项目名称列,不要列名
    df = pd.read_excel(path, usecols=[0, 1], names=None)
    # 生成到list
    df_li = df.values.tolist()
    result = []
    # make内容
    for s_li in df_li:
        str = s_li[0] + '\t' + s_li[1].replace('？', '') + '的答案'
        result.append(str)
    return result


def list_to_file(data):
    """
    把list生成到文件
    """
    with open('sample_docs', 'w') as filehandle:
        filehandle.writelines("%s\n" % place for place in data)


if __name__ == "__main__":
    # 文件路径
    hotel_path = "/Users/huyida/Desktop/hotel.xlsx"
    railway_ticket_path = "/Users/huyida/Desktop/railway.xlsx"
    result_list = []
    # 头标题
    result_list.append("question\tanswer")
    # 清洗格式
    result_list.extend(excel_one_line_to_list(hotel_path))
    result_list.extend(excel_one_line_to_list(railway_ticket_path))
    # 生成FAQ文件
    list_to_file(result_list)
