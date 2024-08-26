import { ConfigService } from '@nestjs/config';

export const MongoConfigFactory = async (config: ConfigService) => {
  const monogConfigs = config.get('mongo');
  return monogConfigs ? monogConfigs : undefined;
};
