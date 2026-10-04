export class PingPong<T> {
  #read: T
  #write: T

  constructor(read: T, write: T) {
    this.#read = read
    this.#write = write
  }

  get read(): T {
    return this.#read
  }

  get write(): T {
    return this.#write
  }

  swap(): void {
    ;[this.#read, this.#write] = [this.#write, this.#read]
  }
}
