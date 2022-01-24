# LeetCode
first version
#!/usr/bin/env python3 -Es
# -*- coding: utf-8 -*-

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
    
    from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import logging
import subprocess


class SafeRemoveCommon:
    def __init__(self):
        pass

    @staticmethod
    def init_log_formate(log_level, log_path):
        """
        设置日志初始化
        :param log_level: 日志级别
        :param log_path: 日志路径
        :return: None
        """
        log_format = '%(asctime)s %(filename)s (%(funcName)s %(lineno)s) [%(levelname)s] - %(message)s'
        logging.basicConfig(format=log_format, datefmt='%Y-%m-%d %H:%M:%S', level=log_level,
                            filename=os.path.join(os.getcwd(), log_path))

    @staticmethod
    def fun_exec_command(cmd):
        """
        用于执行bash调用的函数
        :param cmd: bash调用的命令
        :return: 成功返回0和标准输出，
        失败返回bash命令的返回值和标准出错
        """
        logging.info('cmd:{}'.format(cmd))
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        process.wait()
        stdout, stderr = process.communicate()
        ret_code = process.returncode
        return ret_code, stdout if 0 == ret_code else stderr
        
        from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import re
import os
import sys
import json
import copy
import time
import pathlib
import logging
import argparse
import datetime

from safe_remove import SafeRemove
from safe_remove_common import SafeRemoveCommon

LOG_LEVEL = logging.INFO
# LOG_FILE = "/home/hikos/system/log/hikos_safe_remove.log"
LOG_FILE = " /var/log/safe_remove/hikos_safe_remove.log"

# 实例化共享类，进行调用
safe_remove_common = SafeRemoveCommon()
# 初始化日志格式、日志路径、等级
safe_remove_common.init_log_formate(LOG_LEVEL, LOG_FILE)


class SafeRemoveUI(SafeRemove):
    def __init__(self):
        super(SafeRemoveUI, self).__init__()
        # 提供给用户的配置文件
        # safe_remove.cfg： 配置文件添加说明
        # safe_remove.d: 不同产品所选取的配置文件
        # 默认创建配置文件(暂时不考虑)
        self.default_protect_path = []
        self.is_remove_path = [1]*(len(self.default_protect_path))
        self.cmd_log = "/usr/bin/cli_log -a set -c LOG_SET_OPERATION_LOG -i {} -e {} >/dev/null 2>&1"

    def write_config_file(self, mode, path_list, is_remove_list):
        """
        将（保护路径：是否默认删除标志位）写入配置文件
        :param mode: 写入模式
        :param path_list: 路径
        :return:
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        try:
            with open(self.usr_config_file, mode) as f:
                [f.write("{}:{}\n".format(path, flag)) for path, flag in zip(path_list, is_remove_list)]
        except Exception as e:
            logging.error("analyze file list {} fail for {}".format(path_list, str(e)))

    def remove_config_file(self, path):
        """
        移除文件中（保护路径：是否默认删除标志位）的内容
        :param path: 移除的路径
        :return: None
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        with open(self.usr_config_file, 'r') as f:
            lines = f.readlines()
        with open(self.usr_config_file, 'w') as f:
            for line in lines:
                # if path not in line:
                if path != line.split(':')[0]:
                    f.write(line)

    def is_mount_points(self, path):
        """
        判断是否为挂载点
        :param path: 路径
        :return: 挂载路径
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        path = os.path.abspath(path)
        while path != os.path.sep:
            if os.path.ismount(path):
                return path
            path = os.path.abspath(os.path.join(path, os.pardir))
        return path

    def timestamp_to_time(self, timestamp):
        """
        把时间戳转化为时间: 1479264792 to 2016-11-16 10:53:12
        :param timestamp: 时间戳
        :return: 标准时间
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        timeStruct = time.localtime(timestamp)
        return time.strftime('%Y-%m-%d %H:%M:%S', timeStruct)

    def get_path_create_time(self, filePath):
        """
        获取文件的创建时间
        :param filePath: 路径
        :return: 创建时间
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        t = os.path.getctime(filePath)
        return self.timestamp_to_time(t)

    def get_path_size(self, path):
        """
        获取指定路径的文件夹大小
        :param path: 路径
        :return: 容量（MB）
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        total_size = 0
        if not os.path.exists(path):
            return total_size
        if os.path.isfile(path):
            return round(os.path.getsize(path) / float(1024 * 1024), 2)
        if os.path.isdir(path):
            for root, dirs, files in os.walk(path):
                total_size += sum([os.path.getsize(os.path.join(root, name))
                                   for name in files])
                return round(total_size / float(1024 * 1024), 2)

    def get_path_usage_size(self, path):
        """
        获取指定路径占用空间大小
        :param path: 指定路径
        :return: 容量（Bytes）
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        if not os.path.exists(path):
            return None
        elif os.path.isfile(path):
            return int(os.path.getsize(path))
        else:
            response = os.popen('du -sh {} 2>/dev/null'.format(path))
            str_size = response.read().split()[0]
            response.close()
            f_size = float(re.findall(r'[.\d]+', str_size)[0])
            size_unit = re.findall(r'[A-Z]', str_size)
            size_unit = size_unit[0] if size_unit else ""
            if size_unit == 'M':
                f_size = int(f_size * 1024 ** 2)
            if size_unit == 'G':
                f_size = int(f_size * 1024 ** 3)
            if size_unit == 'T':
                f_size = int(f_size * 1024 ** 4)
            return int(f_size)

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

    def get_mount_paths(self, mount_path):
        """
        获取挂载点mount_point所有的关联路径
        :param mount_point: 挂载点路径
        :return: 关联挂载点列表
        """
        mount_paths, file_paths = map(lambda path: list(path), self.get_mount_points())
        mount_points = dict(zip(mount_paths, file_paths))
        file_system = mount_points[mount_path]
        return [key for key, value in mount_points.items() if value == file_system and key != mount_path] + \
                [mount_points[mount_path]]

    def safe_get(self, args):
        """
        获取保护路径的属性与类型
        :param args: 暂时不需要
        :return: Json数据
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        # 返回Json数据 定义私有变量
        __data = {
            "path": "",
            "flag": "",
            "style": ""
        }
        result_output = {
            "code": 0,
            "msg": "",
            "data": {}
        }
        # 获取配置文件中保护路径的详情
        if os.path.exists(self.usr_config_file):
            protect_path, is_remove_flag = self.read_config_file(self.usr_config_file)
            protect_path = [path for path in protect_path if "" != path]
            is_remove_flag = [flag for flag in is_remove_flag if "" != flag]
            protect_dict = dict(zip(protect_path, is_remove_flag))
            protect_list = sorted(protect_dict.items(), key=lambda k: len(k[0]))
            if 0 != len(protect_list):
                result_output["code"] = 0
                result_output["msg"] = "success"
                for path, flag in protect_list:
                    __data["path"] = path
                    __data["flag"] = flag
                    if os.path.ismount(path):
                        __data["style"] = u"挂载点"
                    elif os.path.islink(path):
                        __data["style"] = u"软链接"
                    elif os.path.isdir(path):
                        __data["style"] = u"普通目录"
                    elif os.path.isfile(path):
                        __data["style"] = u"普通文件"
                    elif "*" in path:
                        __data["style"] = u"通配符路径"
                    else:
                        logging.error(u"不存在此路径")
                    for key, value in __data.items():
                        print("{}:{}".format(key, value))
                    print("\n", end="")
                    result_output["data"] = {u"正常获取保护路径信息"}
            else:
                result_output["code"] = 1
                result_output["msg"] = "fail"
                result_output["data"] = {u"配置文件为空!"}
        else:
            result_output["code"] = 1
            result_output["msg"] = "fail"
            result_output["data"] = {u"不存在此配置文件路径!"}
        # print(json.dumps({"list":result_output["data"]}, indent=4))
        return result_output["code"]

    def safe_get_all(self, args):
        """
        获取保护路径详情
        :param args: 对应路径
        :return: 路径详情（创建时间，占用空间，原路径， 链接关联路径，挂载关联路径）
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        # 返回Json数据
        __data = {
            "create_time": "",
            "path_size": "",
            "real_path": "",
            "link_path": "",
            "mounts_path": ""
        }
        result_output = {
            "code": 0,
            "msg": "",
            "data": {}
        }
        if not args.path:
            result_output["code"] = 1
            result_output["msg"] = "fail"
            result_output["data"] = u"输入路径为空"
        else:
            for path in args.path:
                path = os.path.abspath(path)
                __data["real_path"] = path
                __data["create_time"] = self.get_path_create_time(path)
                # __data["path_size"] = self.get_path_size(path)
                __data["path_size"] = self.get_path_usage_size(path)
                if os.path.ismount(path):
                    # __data["mounts_path"] = mount_points[os.path.abspath(path)]
                    for str_path in self.get_mount_paths(os.path.abspath(path)):
                        __data["mounts_path"] += str_path
                        __data["mounts_path"] += ";"
                elif os.path.islink(path):
                    __data["link_path"] = str(pathlib.Path(path).resolve())
                else:
                    __data["link_path"] = ""
                    __data["mounts_path"] = ""
                for key, value in __data.items():
                    print("{}:{}".format(key, value))
            else:
                result_output["code"] = 0
                result_output["msg"] = "success"
                result_output["data"] = u"正常获取保护路径详情"
        # print(json.dumps({"list": result_output["data"]}, indent=4))
        return result_output["code"]

    def safe_add(self, args):
        """
        用户添加保护路径至配置文件中
        :param args:
        :return:Json数据
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        result_output = {
            "code": 0,
            "msg": "",
            "data": {}
        }
        try:
            # 判断输入路径是否为空
            if not args.path:
                result_output["code"] = 1
                result_output["msg"] = "fail"
                result_output["data"] = u"输入路径为空"
            else:
                add_path, add_flag = [], []
                protect_path, is_remove_flag = self.read_config_file(self.usr_config_file)
                for path in args.path:
                    if '*' in path:
                        add_path.append(path)
                        add_flag.append(0)
                    elif os.path.abspath(path) in protect_path:
                        # 判断保护是否已存在于配置文件
                        result_output["code"] = 1
                        result_output["msg"] = "fail"
                        result_output["data"] = u"{}路径已存在".format(path)
                        ret_code, ret_str = safe_remove_common.fun_exec_command(
                            self.cmd_log.format("0x01080035", path))
                        if 0 != ret_code:
                            logging.error("Record added protect path log fail for {}".format(ret_str))
                        break
                    else:
                        path = os.path.abspath(path)
                        if os.path.exists(path):
                            add_path.append(path)
                            add_flag.append(0)
                            result_output["code"] = 0
                            result_output["msg"] = "success"
                            result_output["data"] = ""
                            ret_code, ret_str = safe_remove_common.fun_exec_command(
                                self.cmd_log.format("0x01080034", path))
                            if 0 != ret_code:
                                logging.error("Record added protect path log fail for {}".format(ret_str))
                        else:
                            result_output["code"] = 1
                            result_output["msg"] = "fail"
                            result_output["data"] = u"{}路径不存在".format(path)
                            ret_code, ret_str = safe_remove_common.fun_exec_command(
                                self.cmd_log.format("0x01080035", path))
                            if 0 != ret_code:
                                logging.error("Record added protect path log fail for {}".format(ret_str))
                else:
                    self.write_config_file('a', add_path, add_flag)
        except Exception as e:
            logging.error("Add protected paths to config file fail for {}".format(str(e)))
            result_output["code"] = 1
            result_output["msg"] = "fail"
            result_output["data"] = str(e)
        return result_output["code"]

    def safe_remove(self, args):
        """
        从配置文件中移除不再需要保护的路径
        :param args: 待移除路径列表
        :return:Json数据
        """
        logging.info('step into method: {}'.format(self.get_method_name()))
        result_output = {
            "code": 0,
            "msg": "",
            "data": {}
        }
        try:
            # 判断输入路径是否为空
            if not args.path:
                result_output["code"] = 1
                result_output["msg"] = "fail"
                result_output["data"] = u"输入路径为空"
            else:
                [self.remove_config_file(path) for path in args.path]
                result_output["code"] = 0
                result_output["msg"] = "success"
                result_output["data"] = ""
                ret_list = [safe_remove_common.fun_exec_command(
                    self.cmd_log.format("0x01080036", path)) for path in args.path]
                ret_code_list = [ret_value[0] for ret_value in ret_list]
                if any(ret_code_list):
                    logging.error("Record remove protect path log fail.")
        except Exception as e:
            logging.error("Remove protected paths to config file fail for {}".format(str(e)))
            result_output["code"] = 1
            result_output["msg"] = "fail"
            result_output["data"] = str(e)
            ret_code, ret_str = safe_remove_common.fun_exec_command(self.cmd_log.format("0x01080037", str(e)))
            if 0 != ret_code:
                logging.error("Record remove protect path log fail for {}".format(ret_str))
        return result_output["code"]


def main(args):
    # 实例化SafeRemove类
    safe_rm_ui = SafeRemoveUI()
    safe_rm_api = dict(get=safe_rm_ui.safe_get, get_all=safe_rm_ui.safe_get_all,
                       add=safe_rm_ui.safe_add, remove=safe_rm_ui.safe_remove)
    try:
        # 选择不同的接口操作，包括safe_get、safe_get_all、safe_add、safe_remove
        return safe_rm_api[args.action](args)
    except KeyError as e:
        logging.error("analysis args fail for {}".format(str(e)))
        return 1
    except Exception as e:
        logging.error("execute function fail for {}".format(str(e)))
        return 2


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="python safe_remove_ui.py", description="safe remove ui")
    # 用户操作: 包括查询get、添加add、移除remove
    parser.add_argument("-a", "--action", type=str, default="", help="action{get|get_all|add|remove}, Get which action to execute")
    # 接收保护路径列表
    parser.add_argument("-p", "--path", type=str, default="", nargs="*", help="delete path list [path...]")
    opt = parser.parse_args()
    sys.exit(main(opt))
    
    import os
import sys
import random
import unittest
import logging
# import HTMLTestRunner
# from unittestreport import TestRunner
sys.path.append("/home/hikos/system/bin/")
from safe_remove import SafeRemove
from safe_remove_ui import SafeRemoveUI
from safe_remove_common import SafeRemoveCommon

safe_remove = SafeRemove()
safe_remove_ui = SafeRemoveUI()
safe_remove_common = SafeRemoveCommon()
record_lines = ["success"]
file_path = "/home/hikos/system/bin/auto_test/safe_remove_auto_test.log"


class Args:
    def __init__(self, path):
        self.path = path
        self.force = ""
        self.recursive = "r"
        self.i = ""
        self.dir = ""


class MyTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        print("full test case begin!\n")

    @classmethod
    def tearDownClass(cls):
        # 记录测试报告
        with open(file_path, "w+") as f:
            [f.write("{}\n".format(content)) for content in record_lines]
        print("successfully full test case end!\n")

    def setUp(self):
        print("this test case begin!")

    def tearDown(self):
        print("successfully this test case end!")

    def auto_create_path(self, nums=0):
        """
        在/home/test下随机创建测试路径
        :return:
        """
        # 给出路径名称列表
        paths = ["test_" + str(i) for i in range(nums)]
        for path in paths:
            cmd = "mkdir -p /home/test/" + path
            ret_code, _ = safe_remove_common.fun_exec_command(cmd)
        return ["/home/test/" + path  for path in paths]

    def test_safe_get(self):
        """
        测试保护路径列表获取
        :return:
        """
        self.assertEqual(safe_remove_ui.safe_get(None), 0)
        record_lines.append("success: 获取默认保护路径成功")

    def test_safe_get_all(self):
        """
        测试获取保护路径详情
        :return:
        """
        protect_path = safe_remove_ui.read_config_file(safe_remove_ui.usr_config_file)[0]
        protect_path.remove('/')  # du -sh / 出现错误
        args = Args(protect_path)
        self.assertEqual(safe_remove_ui.safe_get_all(args), 0)
        record_lines.append("success: 获取保护路径详情成功")

    def test_safe_add_remove(self):
        """
        测试创建->移除->添加->移除->删除
        在/home/test/下随机创建路径测试
        :return:
        """
        args = Args(self.auto_create_path(random.randint(1, 10)))
        # print("safe remove protect path {}".format(args.path))
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        # print("safe add protect path {}".format(args.path))
        self.assertEqual(safe_remove_ui.safe_add(args), 0)
        # print("safe remove protect path {}".format(args.path))
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        # 测试完添加-移除保护路径后需要删除测试路径
        ret_code_del = safe_remove.execute_delete(args)
        self.assertEqual(ret_code_del, 0)
        record_lines.append("success: 创建->移除->添加->移除->删除保护路径成功")

    def test_safe_delete(self):
        """
        测试安全删除功能:创建测试路径->添加测试路径->执行删除测试路径
        测试路径/home/test/，分场景测试：
        (1)普通场景，包括绝对路径与相对路径删除保护路径
        (2)通过进入其他目录删除保护路径
        :return:
        """
        # 场景一(1)(2)
        args = Args(self.auto_create_path(random.randint(1, 10)))
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        self.assertEqual(safe_remove_ui.safe_add(args), 0)
        ret_code_del = safe_remove.execute_delete(args)
        self.assertEqual(ret_code_del, 1)
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        ret_code_del = safe_remove.execute_delete(args)
        self.assertEqual(ret_code_del, 0)
        # self.countTestCases()
        record_lines.append("success: 通过绝对路径删除保护路径测试成功")

    def test_safe_delete_pre(self):
        """
        (3)通过删除保护路径的前级目录删除保护路径
        :return:
        """
        # 一个场景一个测试用例TestCase
        # 所有删除场景为一个suite
        # 包括保护路径和非保护路径
        paths = self.auto_create_path(random.randint(1, 10))
        args = Args(paths)
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        self.assertEqual(safe_remove_ui.safe_add(args), 0)
        # 此处应该遍历所有的前级路径，当前取上一级路径测试
        paths_ = [os.path.abspath(os.path.join(path, os.pardir)) for path in paths]
        args_ = Args(paths_)
        ret_code_del = safe_remove.execute_delete(args_)
        self.assertEqual(ret_code_del, 1)
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        ret_code_del = safe_remove.execute_delete(args)
        self.assertEqual(ret_code_del, 0)
        record_lines.append("success: 通过前级路径删除保护路径测试成功")

    def test_safe_delete_link(self):
        """
        (4)通过保护路径的软链接删除保护路径
        进一步实现通过保护路径的软链接删除其内层的其他保护路径
        这种场景会使保护路径失效，现在此版本的情况是可以防止进入
        软链接进行删除，<但是其他非保护路径亦不可以被删除,（有待
        进一步解决）)>
        :return:
        """
        paths = self.auto_create_path(random.randint(1, 10))
        paths_link = [path + "_link" for path in paths]
        [safe_remove_common.fun_exec_command("ln -s {} {}".format(path, path_link))
         for path, path_link in zip(paths, paths_link)]
        args, args_ = Args(paths_link), Args(paths)
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        self.assertEqual(safe_remove_ui.safe_add(args), 0)
        ret_code_del = safe_remove.execute_delete(args)
        self.assertEqual(ret_code_del, 1)
        ret_code_del = safe_remove.execute_delete(args_)
        self.assertEqual(ret_code_del, 1)
        self.assertEqual(safe_remove_ui.safe_remove(args), 0)
        self.assertEqual(safe_remove_ui.safe_remove(args_), 0)
        ret_code_del = safe_remove.execute_delete(args)
        self.assertEqual(ret_code_del, 0)
        ret_code_del = safe_remove.execute_delete(args_)
        self.assertEqual(ret_code_del, 0)
        record_lines.append("success: 通过软链接路径删除保护路径测试成功")

    def test_safe_delete_mount(self):
        """TODO 考虑是否测试所有的保护挂载点路径，暂不实现
        (5) 通过保护路径的关联挂载点路径删除保护路径
        这种场景下，会使保护路径通过进入其挂载点关联路径删除其他
        保护路径，暂时这种场景未在此版本实现。
        暂时测试挂载点路径：/dev/mapper/centos_hikvisionos-opt
        :return:
        """
        safe_remove_common.fun_exec_command("mkdir -p /home/test/mount_opt")
        safe_remove_common.fun_exec_command("mount /dev/mapper/centos_hikvisionos-opt /home/test/mount_opt")
        ret_code_del = safe_remove.execute_delete(args=Args(["/opt", "/home/test/mount_opt"]))
        self.assertEqual(ret_code_del, 1)
        safe_remove_common.fun_exec_command("umount /home/test/mount_opt")
        ret_code_del = safe_remove.execute_delete(args=Args(["/home/test/mount_opt"]))
        self.assertEqual(ret_code_del, 0)
        record_lines.append("success: 通过挂载点路径删除保护路径测试成功")


if __name__ == '__main__':
    unittest.main(verbosity=2)
