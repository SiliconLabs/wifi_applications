import { Test, TestingModule } from '@nestjs/testing';
import { getModelToken } from '@nestjs/mongoose';
import { SessionService } from '../service/session.service';
import { Session } from '../../../models/session.schema';
import { Model } from 'mongoose';

const mockToken = '21cfe013-0613-4fc0-a3c8-383889c6c4ff';
const testUserEmail = 'test@test.com';
const mockSession = {
  token: mockToken,
  userEmail: testUserEmail,
  expiresAt: new Date(new Date().getTime() + 10000),
};

class mockSessionModel {
  save = jest.fn().mockResolvedValue(mockSession);

  static findOne = jest.fn();
  static deleteMany = jest.fn().mockResolvedValue({ deletedCount: 1 });
}

jest.mock('uuid', () => ({
  v4: jest.fn(() => mockToken),
}));

describe('SessionService', () => {
  let sessionService: SessionService;
  let model: Model<Session>;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        SessionService,
        {
          provide: getModelToken(Session.name),
          useValue: mockSessionModel,
        },
      ],
    }).compile();

    sessionService = module.get<SessionService>(SessionService);
    model = module.get<Model<Session>>(getModelToken(Session.name));
  });

  describe('validateSession', () => {
    it('Success - Should return true if users valid session is found', async () => {
      mockSessionModel.findOne.mockResolvedValueOnce(mockSession);

      const result = await sessionService.validateSession(testUserEmail);
      expect(result).toBe(true);
      expect(mockSessionModel.findOne).toHaveBeenCalledWith({
        userEmail: testUserEmail,
        expiresAt: { $gt: expect.any(Date) },
      });
    });

    it('Failure - Should return false if no users valid session is found', async () => {
      mockSessionModel.findOne.mockResolvedValueOnce(null);

      const result = await sessionService.validateSession(testUserEmail);
      expect(result).toBe(false);
      expect(mockSessionModel.findOne).toHaveBeenCalledWith({
        userEmail: testUserEmail,
        expiresAt: { $gt: expect.any(Date) },
      });
    });
  });

  describe('checkActiveSession', () => {
    it('Success - Should return true if a valid session is found', async () => {
      mockSessionModel.findOne.mockResolvedValueOnce(mockSession);

      const result = await sessionService.checkActiveSession();
      expect(result).toBe(true);
      expect(mockSessionModel.findOne).toHaveBeenCalledWith({
        expiresAt: { $gt: expect.any(Date) },
      });
    });

    it('Failure - Should return false if no valid session is found', async () => {
      mockSessionModel.findOne.mockResolvedValueOnce(null);

      const result = await sessionService.checkActiveSession();
      expect(result).toBe(false);
      expect(mockSessionModel.findOne).toHaveBeenCalledWith({
        expiresAt: { $gt: expect.any(Date) },
      });
    });
  });

  describe('deleteSession', () => {
    it('should delete sessions for a given user email', async () => {
      await sessionService.deleteSession();
      expect(mockSessionModel.deleteMany).toHaveBeenCalledWith({});
    });
  });

  describe('createSession', () => {
    it('should create a new session', async () => {
      const expiresAt = new Date();
      expiresAt.setHours(expiresAt.getHours() + 1);
      const result = await sessionService.createSession(testUserEmail);
      expect(result instanceof Date).toBe(true);
    });
  });
});
