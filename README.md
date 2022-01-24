# LeetCode
first version
#!/usr/bin/env python3 -Es
# -*- coding: utf-8 -*-
# author: Hongxiang Chai

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import sys
import glob
import logging
import pathlib
import argparse
import inspect

from safe_remove_common import SafeRemoveCommon

LOG_LEVEL = logging.INFO
# LOG_FILE = "/home/hikos/system/log/hikos_safe_remove.log"
LOG_FILE = "/var/log/safe_remove/hikos_safe_remove.log"

# 实例化共享类，进行调用
safe_remove_common = SafeRemoveCommon()
# 初始化日志格式、日志路径、等级
safe_remove_common.init_log_formate(LOG_LEVEL, LOG_FILE)


class SafeRemove:
    def __init__(self):
        # 提供给用户的配置文件
        # safe_remove.cfg:  配置文件添加说明
        # safe_remove.d:    不同产品所选取的配置文件
        self.usr_config_file = "/home/hikos/system/config/safe_remove.d/safe_remove_hikos.cfg"
        self.is_has_config_file(self.usr_config_file)

    def is_has_config_file(self, path):
        if os.path.exists(path):
            logging.info("config file: {} exists".format(path))
        else:
            logging.info("config file: {} is not exists".format(path))

    def get_method_name(self):
        """
        获取正在运行函数(或方法)名称
        :return: 方法名
        """
        return inspect.stack()[1][3]

    def read_config_file(self, path):
        """
        读取配置文件内容
        :param path: 文件路径
        :return: 保护路径列表
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        path_list, is_remove_list = [], []
        try:
            with open(path, 'a+') as f:
                f.seek(0)  # 移动到指针开头
                for line in f:
                    path_list.append(line.strip('\n').split(":")[0]) if "\n" != line else None
                    is_remove_list.append(line.strip('\n').split(":")[1]) if "\n" != line else None
        except Exception as e:
            logging.error("analyze file {} fail for {}".format(path, str(e)))
        finally:
            return path_list, is_remove_list

    def pre_link(self, path):
        """
        前级是否存在软链接路径
        :param path: 路径
        :return: 存在的软链接路径
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        path = os.path.abspath(path)
        while path != os.path.sep:
            if os.path.islink(path):
                return path
            path = os.path.abspath(os.path.join(path, os.pardir))
        return path

    def get_mount_points(self):
        """
        获取系统所有的挂载点及其对应的文件系统
        :return: 挂载点，文件系统
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        cmd = "mount -v"
        ret_code, ret_str = safe_remove_common.fun_exec_command(cmd)
        if 0 != ret_code:
            logging.error("get mount points fail for {}".format(ret_str))
        lines = ret_str.decode('utf-8').split('\n')
        points = map(lambda line: line.split()[2], [line for line in lines if line])
        filesystem = map(lambda line: line.split()[0], [line for line in lines if line])
        return points, filesystem

    def filter_path(self, delete_paths, all_paths):
        """
        过滤可删除路径
        :param path: 待删除路径
        :return: 过滤待删除路径
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        delete_path = ""
        # 判断输入路径是否在保护路径列表中，
        # 如果不在则保留， 如果存在则置为空
        for path in delete_paths:
            if path not in all_paths:
                delete_path += path
                delete_path += " "
            else:
                delete_path += ""
                print("safe-rm: skipping {}".format(path))
                logging.info("{} is skipping.".format(path))
        return delete_path

    def list_dir(self, input_path):
        """
        获取路径下非隐藏文件或路径
        :return: 路径列表
        """
        return [path for path in os.listdir(input_path) if "." != path[0]]

    def execute_delete(self, args):
        """
        需要设计的功能主要包括以下四点：
        (1) 防止进入保护目录执行rm -rf *进行删除
        (2) 防止删除被保护的软链接文件以及源文件
        (3) 防止删除被保护挂载点/proc/self/mounts及关联路径
        (4) 暂时覆盖不到的场景：配置路径/home/test 执行rm -rf /home(已解决)
                               源文件对应的所有软链接无法获取(边界条件)
        执行 /usr/bin/rm 删除一个或多个文件
        :return: 执行成功返回0和标准输出，失败返回bash命令的返回值和标准出错
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        parameter, path_list, process_full_paths = "", [], []
        full_paths = self.read_config_file(self.usr_config_file)[0]

        # 保护路径内的内容不允许被删除（仅下一级递归）
        # wildcards_paths = [path + "//*" for path in full_paths if os.path.isdir(path)]
        # file_paths = [path for path in full_paths if os.path.isfile(path)]

        wildcards_paths = [glob.glob(path) for path in full_paths]
        for i in range(len(wildcards_paths)):
            process_full_paths += wildcards_paths[i]
        # process_full_paths = [os.path.abspath(path) for path in process_full_paths]
        # process_full_paths += file_paths

        # 添加保护路径对应的链接路径
        protect_link_paths = [str(pathlib.Path(path).resolve()) for path in process_full_paths if os.path.islink(path)]
        process_full_paths += protect_link_paths
        process_full_paths = list(set(process_full_paths))

        # 获取系统所有的挂载点及其对应的文件系统
        mount_paths, file_paths = map(lambda path: list(path), self.get_mount_points())
        mount_points = dict(zip(mount_paths, file_paths))
        file_system_path = [mount_points[path] for path in process_full_paths if os.path.ismount(path)]

        # 实现/usr/bin/rm原生参数
        if args.force or args.i or args.recursive or args.dir:
            parameter += "-"
            parameter += "f" if args.force else ""
            parameter += "i" if args.i else ""
            parameter += "r" if args.recursive else ""
            parameter += "d" if args.dir else ""
        else:
            logging.debug("options are not finding")
        parameter += " "

        # 单独删除保护路径下的一个仍至多个文件
        # 思路：利用哈希表统计保护列表中保护路径对应的所有输入路径
        hash_dict, hash_num, input_paths, post_input_paths = {}, {}, [], []
        input_paths = list(set(args.path))
        # (1) 将不在保护路径里的文件或路径置于删除列表中
        for path in input_paths:
            pre_path = os.path.abspath(os.path.join(path, os.pardir))
            if pre_path not in process_full_paths:
                post_input_paths.append(path)
            else:
                logging.debug("{} is in protect path".format(path))
        # (2) 利用hash表将输入路径根据保护列表归类
        for path in process_full_paths:
            if os.path.isdir(path):
                hash_dict[path] = [inner_path for inner_path in input_paths if os.path.abspath(
                    os.path.join(inner_path, os.pardir)) == path]
                hash_num[path] = len(self.list_dir(path))
            else:
                logging.debug("{} is file".format(path))
        # (3) 将保护路径中包含所有内容的键值移除
        for key, value in hash_num.items():
            if value == len(hash_dict[key]) and 0 != value:
                print("safe-rm: skipping {}/*".format(key))
            else:
                post_input_paths += hash_dict[key]

        # 输入路径列表， 解决问题（4）
        for path in post_input_paths:
            flags = False  # 前级递归子序列是否存在标志位
            # 防止进入保护路径执行删除
            absolute_path = os.path.abspath(path)
            # 只要输入路径为某一保护路径的前级递归子序列均不可执行删除操作
            # 此处的优化：只要找到子序列立刻停止搜索
            for _path_ in process_full_paths:
                __path__ = _path_
                while __path__ != os.path.sep:
                    if __path__ == absolute_path:
                        print("safe-rm: skipping {}".format(path))
                        flags = True
                        break
                    else:
                        logging.debug("pre path is not finding")
                    __path__ = os.path.abspath(os.path.join(__path__, os.pardir))
            if not flags:
                # 防止通过软链接删除保护路径，ex. /home/test_link->/home/test
                # 通过rm -rf /home/test_link_test 删除/home/test/test
                pre_link_path = self.pre_link(absolute_path)
                if os.path.islink(pre_link_path) and pre_link_path in process_full_paths:
                    print("safe-rm: skipping {}".format(path))
                    logging.info("{} is protect link path in process_full_paths.".format(path))
                # 防止删除挂载点对应的文件系统路径
                elif absolute_path in file_system_path:
                    print("safe-rm: skipping {}".format(path))
                    logging.info("{} is file system in process_full_paths.".format(path))
                # 判断挂载点对应的是不是同一文件系统防止通过关联挂载点路径删除保护挂载点路径
                elif os.path.ismount(absolute_path) or absolute_path in mount_paths:
                    mount_list = []
                    file_system = mount_points[absolute_path]
                    for mount_path in process_full_paths:
                        if os.path.ismount(mount_path):
                            mount_list.append(mount_points[mount_path])
                        else:
                            logging.debug("{} is not mount point".format(mount_path))
                    else:
                        logging.debug("loop normal is end")
                    if file_system in mount_list:
                        print("safe-rm: skipping {}".format(path))
                        logging.info("{} is mounts in process_full_paths.".format(path))
                    else:
                        logging.debug("{} is not in mount list".format(file_system))
                else:
                    path_list.append(absolute_path)
                if args.i:
                    print("/usr/bin/rm remove '{}'?".format(path))
                else:
                    logging.debug("parameter 'i' is not exists")
            else:
                logging.info(u"待删除路径为配置文件中保护路径的子序列，不允许执行删除！")
        cmd = "/usr/bin/rm " + parameter + self.filter_path(path_list, process_full_paths)
        ret_code, ret_str = safe_remove_common.fun_exec_command(cmd)
        if 0 != ret_code:
            logging.error("standard error: {}; return code {}".format(ret_str, ret_code))
            print(ret_str.decode('utf-8'), end="")
            return 1
        else:
            logging.debug("executing rm is normal")
        return 0


def main(args):
    # 实例化SafeRemove类
    safe_rm = SafeRemove()
    try:
        return safe_rm.execute_delete(args)
    except KeyError as e:
        logging.error("analysis args fail for {}".format(str(e)))
        return 1
    except Exception as e:
        logging.error("execute function fail for {}".format(str(e)))
        return 2


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="python safe_remove.py", description="safe remove")
    parser.add_argument("path", type=str, default="", nargs="*", help="delete path list [path...]")
    parser.add_argument("-f", "--force", action='store_true', help="ignore nonexistent files and arguments, never prompt")
    parser.add_argument("-i", action='store_true', help="prompt before every removal")
    parser.add_argument("-r", "-R", "--recursive", action='store_true', help="remove directories and their contents recursively")
    parser.add_argument("-d", "--dir", action='store_true', help="remove empty directories")
    opt = parser.parse_args()
    sys.exit(main(opt))
