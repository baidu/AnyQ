## AnyQ solr一键启动

sh solr/anyq_solr.sh solr/sample_faq

配置要求：
    - jdk1.8以上，python2.7
    - 获取anyq定制solr-4.10.3

### solr_deply.sh 接口方法
↓↓**启动solr服务**↓↓
```
sh solr_deply.sh start solr_home solr_port
```

↓↓**停止solr服务**↓↓
```
sh solr_deply.sh stop solr_home solr_port
```

### solr_tools.py 接口方法
↓↓**添加引擎**↓↓
```
add_engine(host, enginename, port=8983, shard=1, replica=1, maxshardpernode=5, conf='myconf')
```

↓↓**删除引擎**↓↓
```
delete_engine(host, enginename, port=8983)
```

↓↓**设置引擎的数据格式**↓↓
```
set_engine_schema(host, enginename, schema_config, port=8983)
chema_config 可以为json文件路径，也可以是一个json list
```

↓↓**文档灌库**↓↓
```
upload_documents(host, enginename, port=8983, documents="", num_thread=1)
```

↓↓**清空库**↓↓
```
clear_documents(host, enginename, port=8983)
```

### solr_tools.py 命令行方式
```
查看命令行使用方法
python solr_tools.py -help
```
