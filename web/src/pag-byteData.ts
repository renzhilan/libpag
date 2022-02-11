import { PAG } from './types';

export class ByteData {
  public static module: PAG;
 

  public pagByteDataWasm;

  public constructor(pagByteDataWasm) {
    this.pagByteDataWasm = pagByteDataWasm;
  }
  /**
   * Returns the width in pixels.
   */
  public async data(): Promise<number> {
    return (await ByteData.module.webAssemblyQueue.exec(this.pagByteDataWasm._data, this.pagByteDataWasm)) as number;
  }
  /**
   * Returns the height in pixels.
   */
  public async length(): Promise<number> {
    return (await ByteData.module.webAssemblyQueue.exec(this.pagByteDataWasm._length, this.pagByteDataWasm)) as number;
  }
}