import json
import os
import sys
from importlib.util import spec_from_file_location, module_from_spec 
import argparse
from pathlib import Path

current_path = os.path.dirname(__file__)

def main():

    data_path = os.path.join(current_path,'..','data')

    # Reads dataset file as cmd line argument
    parser = argparse.ArgumentParser(description="Convert a dataset in json format")
    parser.add_argument("dataset", metavar="dataset", type=str, help="the python file to process. Only the name")
    args = parser.parse_args()
    
    # Loads module and dataset specification in it
    try:
        spec = spec_from_file_location("dataset","/mmpose/configs/_base_/datasets/" + args.dataset)  
        dataset_module = module_from_spec(spec)
        sys.modules["dataset"] = dataset_module
        spec.loader.exec_module(dataset_module)
    except:
        print("Specified dataset does not exist or is ill formatted. Check examples in /mmpose/configs/_base_/datasets")
        exit(-1)

    keypoint_info = dataset_module.dataset_info["keypoint_info"]
    keypoint_json = json.dumps(keypoint_info,ensure_ascii=False)
    
    dataset_base_name = args.dataset.split(".py")[0]
    
    Path(data_path).mkdir(parents=True,exist_ok=True)

    with open(os.path.join(data_path,dataset_base_name+'.json'),'w',encoding='utf-8') as f:
        f.write(keypoint_json)

if __name__ == "__main__":
    main()