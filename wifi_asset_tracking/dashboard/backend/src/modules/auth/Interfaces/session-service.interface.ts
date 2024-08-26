export interface SessionServiceInterface {
  createSession(userEmail: string): Promise<Date>;

  validateSession(userEmail: string): Promise<boolean>;

  checkActiveSession(): Promise<boolean>;

  deleteSession(userEmail: string): Promise<void>;
}
export default SessionServiceInterface;
